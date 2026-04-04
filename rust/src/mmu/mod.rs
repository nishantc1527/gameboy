mod io;
mod mbc1;
mod mbc2;
mod mbc3;
mod mbc5;
mod no_mbc;

use std::{fs::File, io::Read, path::Path};

pub struct Mmu {
    rom_title: String,
    cart_type: u8,
    rom_size: u8,
    ram_size: u8,
    vram: Vec<u8>,
    vram_bank1: Vec<u8>,
    vram_bank_sel: u8,
    wram: Vec<u8>,
    wram_banks: Vec<u8>,
    wram_bank: u8,
    oam: Vec<u8>,
    hram: Vec<u8>,
    boot_active: bool,
    cgb_speed: u8,
    brom: Vec<u8>,
    rom: Vec<u8>,
    extern_ram: Vec<u8>,
    rom_bank: u8,
    rom_bank_hi: u8,
    ram_bank: u8,
    ram_enable: bool,
    mbc1_1mb_mode: bool,
    mbc1_multicart: bool,
    rtc_latched: [u8; 5],
    rtc_latch_state: u8,
    rtc_s: u8,
    rtc_m: u8,
    rtc_h: u8,
    rtc_dl: u8,
    rtc_dh_bit: u8,
    rtc_frac_cycles: u64,
    rtc_halted: bool,
    rtc_carry: bool,
    cgb_mode: bool,
    cgb_compat: bool,
    boot_skipped: bool,
}

#[allow(clippy::manual_range_patterns)]
impl Mmu {
    pub fn new(
        rom_file_name: &str,
        dmg_boot: Option<&[u8]>,
        cgb_boot: Option<&[u8]>,
    ) -> Option<Mmu> {
        let mut rom_title = String::new();
        let vram = vec![0u8; 0x2000];
        let wram = vec![0u8; 0x1000];
        let oam = vec![0u8; 0xA0];
        let hram = vec![0u8; 0x80];
        let mut brom = vec![0u8; 0x900];
        let mut rom = vec![0u8; 0x800000];
        let extern_ram = vec![0u8; 0x20000];
        let ram_bank: u8 = 0;
        let mut mbc1_1mb_mode = false;
        let mut mbc1_multicart = false;
        let mut rom_file = match File::open(Path::new(rom_file_name)) {
            Ok(f) => f,
            Err(_) => {
                eprintln!("ROM not found: \"{}\"", rom_file_name);
                return None;
            }
        };
        let rom_bytes_read = rom_file.read(&mut rom).unwrap_or(0);
        if rom_bytes_read < 0x150 {
            eprintln!(
                "ROM is too small or corrupted ({} bytes, need at least 0x150)",
                rom_bytes_read
            );
            return None;
        }
        let cgb_flag = rom[0x0143] & 0x80 != 0;
        let old_licensee_nintendo = rom[0x014B] == 0x01;
        let new_licensee_nintendo =
            rom[0x014B] == 0x33 && rom[0x0144] == b'0' && rom[0x0145] == b'1';
        let cgb_mode = cgb_flag || old_licensee_nintendo || new_licensee_nintendo;
        let boot_data = if cgb_mode { cgb_boot } else { dmg_boot };
        let expected_size = if cgb_mode { 0x900usize } else { 0x100usize };
        let boot_skipped = if let Some(data) = boot_data {
            if data.len() != expected_size {
                eprintln!(
                    "Boot ROM has wrong size: expected {}, got {}",
                    expected_size,
                    data.len()
                );
                return None;
            }
            brom[..data.len()].copy_from_slice(data);
            false
        } else {
            true
        };
        let mut checksum: u8 = 0u8;
        for byte in rom.iter().take(0x014C + 1).skip(0x0134usize) {
            checksum = checksum.wrapping_sub(*byte).wrapping_sub(1);
        }
        if checksum != rom[0x014D] {
            eprintln!(
                "WARNING: ROM header checksum mismatch (computed 0x{:02X}, header has 0x{:02X})",
                checksum, rom[0x014D]
            );
        }
        (0x0134usize..=0x0142usize).for_each(|i| {
            rom_title.push(rom[i] as char);
        });
        rom_title.push('\0');
        let cart_type: u8 = rom[0x0147];
        let rom_size: u8 = rom[0x0148];
        let ram_size: u8 = rom[0x0149];
        match cart_type {
            0x00 | 0x01 | 0x02 | 0x03 | 0x05 | 0x06 | 0x0F | 0x10 | 0x11 | 0x12 | 0x13 | 0x19
            | 0x1A | 0x1B | 0x1C | 0x1D | 0x1E => (),
            _ => {
                eprintln!("Unsupported cartridge type: 0x{:02X}", cart_type);
                return None;
            }
        }
        match rom_size {
            0x00 | 0x01 | 0x02 | 0x03 | 0x04 | 0x05 | 0x06 | 0x07 | 0x08 => (),
            _ => {
                eprintln!("UNIMPLEMENTED ROM SIZE: ${:02X}\n", rom_size);
                return None;
            }
        }
        match ram_size {
            0x00 | 0x02 | 0x03 | 0x04 => (),
            _ => {
                eprintln!("UNIMPLEMENTED RAM SIZE: ${:02X}\n", ram_size);
                return None;
            }
        }
        let rom_bank: u8 = 1;
        let rom_bank_hi: u8 = 0;
        let ram_enable = false;
        match cart_type {
            0x01 | 0x02 | 0x03 => {
                mbc1_1mb_mode = false;
                mbc1_multicart = mbc1::check_multicart(&rom, rom_size);
                if rom_size > 0x06 {
                    eprintln!("ROM SIZE NOT AVAILABLE\n");
                    return None;
                }
                if ram_size > 0x03 {
                    eprintln!("RAM SIZE NOT AVAILABLE\n");
                    return None;
                }
            }
            0x05 | 0x06 if rom_size > 0x03 => {
                eprintln!("ROM SIZE NOT AVAILABLE\n");
                return None;
            }
            0x05 | 0x06 => {}
            0x0F | 0x10 | 0x11 | 0x12 | 0x13 => {
                if rom_size > 0x07 {
                    eprintln!("ROM SIZE NOT AVAILABLE\n");
                    return None;
                }
                if ram_size > 0x03 {
                    eprintln!("RAM SIZE NOT AVAILABLE\n");
                    return None;
                }
            }
            0x19 | 0x1A | 0x1B | 0x1C | 0x1D | 0x1E => {
                if rom_size > 0x08 {
                    eprintln!("ROM SIZE NOT AVAILABLE\n");
                    return None;
                }
                if ram_size > 0x04 {
                    eprintln!("RAM SIZE NOT AVAILABLE\n");
                    return None;
                }
            }
            _ => (),
        }
        let mmu = Mmu {
            rom_title,
            cart_type,
            rom_size,
            ram_size,
            vram,
            vram_bank1: vec![0u8; 0x2000],
            vram_bank_sel: 0,
            wram,
            wram_banks: vec![0u8; 0x7000],
            wram_bank: 1,
            oam,
            hram,
            boot_active: !boot_skipped,
            cgb_speed: 0,
            brom,
            rom,
            extern_ram,
            rom_bank,
            rom_bank_hi,
            ram_bank,
            ram_enable,
            mbc1_1mb_mode,
            mbc1_multicart,
            rtc_latched: [0u8; 5],
            rtc_latch_state: 0,
            rtc_s: 0,
            rtc_m: 0,
            rtc_h: 0,
            rtc_dl: 0,
            rtc_dh_bit: 0,
            rtc_frac_cycles: 0,
            rtc_halted: false,
            rtc_carry: false,
            cgb_mode,
            cgb_compat: cgb_mode && !cgb_flag,
            boot_skipped,
        };
        Some(mmu)
    }

    pub fn read_extern_ram(&self, loc: u16) -> u8 {
        self.extern_ram[loc as usize]
    }

    pub fn write_extern_ram(&mut self, loc: u16, val: u8) {
        self.extern_ram[loc as usize] = val;
    }

    pub fn get_rom_title(&self) -> &String {
        &self.rom_title
    }

    pub fn is_cgb(&self) -> bool {
        self.cgb_mode
    }

    pub fn is_cgb_compat(&self) -> bool {
        self.cgb_compat
    }

    pub fn read_vram_bank0(&self, addr: u16) -> u8 {
        self.vram[(addr - 0x8000) as usize]
    }

    pub fn read_vram_bank1(&self, addr: u16) -> u8 {
        self.vram_bank1[(addr - 0x8000) as usize]
    }

    pub fn boot_skipped(&self) -> bool {
        self.boot_skipped
    }

    pub fn read_vram(&self, addr: u16) -> u8 {
        if self.cgb_mode && self.vram_bank_sel & 1 == 1 {
            self.vram_bank1[(addr - 0x8000) as usize]
        } else {
            self.vram[(addr - 0x8000) as usize]
        }
    }

    pub fn write_vram(&mut self, addr: u16, val: u8) {
        if self.cgb_mode && self.vram_bank_sel & 1 == 1 {
            self.vram_bank1[(addr - 0x8000) as usize] = val;
        } else {
            self.vram[(addr - 0x8000) as usize] = val;
        }
    }

    pub fn set_vram_bank(&mut self, bank: u8) {
        self.vram_bank_sel = bank & 1;
    }

    pub fn get_vram_bank(&self) -> u8 {
        (self.vram_bank_sel & 1) | 0xFE
    }

    pub fn read_wram(&self, addr: u16) -> u8 {
        if addr < 0xD000 {
            self.wram[(addr - 0xC000) as usize]
        } else if self.cgb_mode {
            self.wram_banks[self.wram_bank as usize * 0x1000 + (addr - 0xD000) as usize]
        } else {
            self.wram_banks[(addr - 0xD000) as usize]
        }
    }

    pub fn write_wram(&mut self, addr: u16, val: u8) {
        if addr < 0xD000 {
            self.wram[(addr - 0xC000) as usize] = val;
        } else if self.cgb_mode {
            self.wram_banks[self.wram_bank as usize * 0x1000 + (addr - 0xD000) as usize] = val;
        } else {
            self.wram_banks[(addr - 0xD000) as usize] = val;
        }
    }

    pub fn set_wram_bank(&mut self, bank: u8) {
        self.wram_bank = if bank & 7 == 0 { 1 } else { bank & 7 };
    }

    pub fn get_wram_bank(&self) -> u8 {
        self.wram_bank | 0xF8
    }

    pub fn read_oam(&self, offset: u16) -> u8 {
        self.oam[offset as usize]
    }

    pub fn write_oam(&mut self, offset: u16, val: u8) {
        self.oam[offset as usize] = val;
    }

    pub fn read_hram(&self, offset: u16) -> u8 {
        self.hram[offset as usize]
    }

    pub fn write_hram(&mut self, offset: u16, val: u8) {
        self.hram[offset as usize] = val;
    }

    pub fn read_boot(&self, addr: u16) -> u8 {
        self.brom[addr as usize]
    }

    pub fn boot_active(&self) -> bool {
        self.boot_active
    }

    pub fn disable_boot(&mut self) {
        self.boot_active = false;
    }

    pub fn read_cgb_speed(&self) -> u8 {
        self.cgb_speed
    }

    pub fn write_cgb_speed(&mut self, val: u8) {
        self.cgb_speed = val;
    }

    pub fn read_rom_region(&self, addr: u16) -> u8 {
        match self.cart_type {
            0x00 => self.no_mbc_read_rom(addr),
            0x01 | 0x02 | 0x03 => self.mbc1_read_rom(addr),
            0x05 | 0x06 => self.mbc2_read_rom(addr),
            0x0F | 0x10 | 0x11 | 0x12 | 0x13 => self.mbc3_read_rom(addr),
            0x19 | 0x1A | 0x1B | 0x1C | 0x1D | 0x1E => self.mbc5_read_rom(addr),
            _ => 0xFF,
        }
    }

    pub fn write_rom_region(&mut self, addr: u16, val: u8) {
        match self.cart_type {
            0x00 => self.no_mbc_write_rom(addr, val),
            0x01 | 0x02 | 0x03 => self.mbc1_write_rom(addr, val),
            0x05 | 0x06 => self.mbc2_write_rom(addr, val),
            0x0F | 0x10 | 0x11 | 0x12 | 0x13 => self.mbc3_write_rom(addr, val),
            0x19 | 0x1A | 0x1B | 0x1C | 0x1D | 0x1E => self.mbc5_write_rom(addr, val),
            _ => {}
        }
    }

    pub fn read_eram_region(&self, addr: u16) -> u8 {
        match self.cart_type {
            0x00 => self.no_mbc_read_ram(addr),
            0x01 | 0x02 | 0x03 => self.mbc1_read_ram(addr),
            0x05 | 0x06 => self.mbc2_read_ram(addr),
            0x0F | 0x10 | 0x11 | 0x12 | 0x13 => self.mbc3_read_ram(addr),
            0x19 | 0x1A | 0x1B | 0x1C | 0x1D | 0x1E => self.mbc5_read_ram(addr),
            _ => 0xFF,
        }
    }

    pub fn write_eram_region(&mut self, addr: u16, val: u8) {
        match self.cart_type {
            0x00 => self.no_mbc_write_ram(addr, val),
            0x01 | 0x02 | 0x03 => self.mbc1_write_ram(addr, val),
            0x05 | 0x06 => self.mbc2_write_ram(addr, val),
            0x0F | 0x10 | 0x11 | 0x12 | 0x13 => self.mbc3_write_ram(addr, val),
            0x19 | 0x1A | 0x1B | 0x1C | 0x1D | 0x1E => self.mbc5_write_ram(addr, val),
            _ => {}
        }
    }
}
