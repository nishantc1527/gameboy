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
    io_regs: [u8; 0x80],
    brom: Vec<u8>,
    rom: Vec<u8>,
    extern_ram: Vec<u8>,
    rom_bank: u8,
    rom_bank_hi: u8,
    ram_bank: u8,
    ram_enable: bool,
    mbc1_1mb_mode: bool,
    mbc1_multicart: bool,
    serial_byte_pending: bool,
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
    joypad_btns: u8,
    joypad_dirs: u8,
    cgb_mode: bool,
    cgb_compat: bool,
    div_reset_pending: bool,
    boot_skipped: bool,
    hdma_active: bool,
    hdma_remaining: u8,
    hdma_src: u16,
    hdma_dst: u16,
}

impl Mmu {
    fn region_read_raw(&self, loc: u16) -> u8 {
        match loc {
            0x8000..0xA000 => {
                if self.cgb_mode && self.vram_bank_sel & 1 == 1 {
                    self.vram_bank1[(loc - 0x8000) as usize]
                } else {
                    self.vram[(loc - 0x8000) as usize]
                }
            }
            0xC000..0xD000 => self.wram[(loc - 0xC000) as usize],
            0xD000..0xE000 => {
                if self.cgb_mode {
                    self.wram_banks[self.wram_bank as usize * 0x1000 + (loc - 0xD000) as usize]
                } else {
                    self.wram_banks[(loc - 0xD000) as usize]
                }
            }
            0xE000..=0xFDFF => self.region_read_raw(loc - 0x2000),
            0xFE00..0xFEA0 => self.oam[(loc - 0xFE00) as usize],
            0xFF00..0xFF80 => self.io_regs[(loc - 0xFF00) as usize],
            0xFF80..=0xFFFF => self.hram[(loc - 0xFF80) as usize],
            _ => 0xFF,
        }
    }

    fn region_write_raw(&mut self, loc: u16, val: u8) {
        match loc {
            0x8000..0xA000 => {
                if self.cgb_mode && self.vram_bank_sel & 1 == 1 {
                    self.vram_bank1[(loc - 0x8000) as usize] = val;
                } else {
                    self.vram[(loc - 0x8000) as usize] = val;
                }
            }
            0xC000..0xD000 => self.wram[(loc - 0xC000) as usize] = val,
            0xD000..0xE000 => {
                if self.cgb_mode {
                    self.wram_banks[self.wram_bank as usize * 0x1000 + (loc - 0xD000) as usize] =
                        val;
                } else {
                    self.wram_banks[(loc - 0xD000) as usize] = val;
                }
            }
            0xE000..=0xFDFF => self.region_write_raw(loc - 0x2000, val),
            0xFE00..0xFEA0 => self.oam[(loc - 0xFE00) as usize] = val,
            0xFF00..0xFF80 => self.io_regs[(loc - 0xFF00) as usize] = val,
            0xFF80..=0xFFFF => self.hram[(loc - 0xFF80) as usize] = val,
            _ => {}
        }
    }
}

#[allow(clippy::manual_range_patterns)]
impl Mmu {
    pub fn new(rom_file_name: &str, boot_rom_file_name: &str) -> Option<Mmu> {
        let mut rom_title = String::new();
        let vram = vec![0u8; 0x2000];
        let wram = vec![0u8; 0x1000];
        let oam = vec![0u8; 0xA0];
        let hram = vec![0u8; 0x80];
        let io_regs = [0u8; 0x80];
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
        let actual_boot_rom = if cgb_mode {
            let p = Path::new(boot_rom_file_name);
            let dir = p.parent().unwrap_or(Path::new("."));
            dir.join("cgb_boot.bin").to_string_lossy().into_owned()
        } else {
            boot_rom_file_name.to_owned()
        };
        let boot_skipped = if boot_rom_file_name.is_empty() {
            true
        } else {
            match File::open(Path::new(&actual_boot_rom)) {
                Ok(mut f) => {
                    let brom_bytes = f.read(&mut brom).ok()?;
                    if cgb_mode && brom_bytes != 0x900 {
                        eprintln!("CGB boot ROM must be 0x900 bytes (got {})", brom_bytes);
                        return None;
                    } else if !cgb_mode && brom_bytes != 0x100 {
                        eprintln!("COULD NOT READ FULL BOOT ROM");
                        return None;
                    }
                    false
                }
                Err(_) => {
                    eprintln!(
                        "Boot ROM not found: \"{}\". Running without boot ROM.",
                        actual_boot_rom
                    );
                    true
                }
            }
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
            0x05 | 0x06 => {
                if rom_size > 0x03 {
                    eprintln!("ROM SIZE NOT AVAILABLE\n");
                    return None;
                }
            }
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
        let mut mmu = Mmu {
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
            io_regs,
            brom,
            rom,
            extern_ram,
            rom_bank,
            rom_bank_hi,
            ram_bank,
            ram_enable,
            mbc1_1mb_mode,
            mbc1_multicart,
            serial_byte_pending: false,
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
            joypad_btns: 0,
            joypad_dirs: 0,
            cgb_mode,
            cgb_compat: cgb_mode && !cgb_flag,
            div_reset_pending: false,
            boot_skipped,
            hdma_active: false,
            hdma_remaining: 0,
            hdma_src: 0,
            hdma_dst: 0x8000,
        };
        if cgb_mode {
            mmu.io_regs[0x4D] = 0;
        }
        if boot_skipped {
            mmu.post_boot_init();
        }
        Some(mmu)
    }

    #[allow(clippy::identity_op)]
    pub fn r_mem(&self, loc: u16) -> u8 {
        if self.io_regs[0x50] == 0 {
            if loc < 0x100 {
                return self.brom[loc as usize];
            }
            if self.cgb_mode && (0x0200..0x0A00).contains(&loc) {
                return self.brom[loc as usize];
            }
        }
        match loc {
            ..0x8000 => match self.cart_type {
                0x00 => self.no_mbc_read_rom(loc),
                0x01 | 0x02 | 0x03 => self.mbc1_read_rom(loc),
                0x05 | 0x06 => self.mbc2_read_rom(loc),
                0x0F | 0x10 | 0x11 | 0x12 | 0x13 => self.mbc3_read_rom(loc),
                0x19 | 0x1A | 0x1B | 0x1C | 0x1D | 0x1E => self.mbc5_read_rom(loc),
                _ => 0xFF,
            },
            0xA000..0xC000 => match self.cart_type {
                0x00 => self.no_mbc_read_ram(loc),
                0x01 | 0x02 | 0x03 => self.mbc1_read_ram(loc),
                0x05 | 0x06 => self.mbc2_read_ram(loc),
                0x0F | 0x10 | 0x11 | 0x12 | 0x13 => self.mbc3_read_ram(loc),
                0x19 | 0x1A | 0x1B | 0x1C | 0x1D | 0x1E => self.mbc5_read_ram(loc),
                _ => 0xFF,
            },
            mut loc => {
                if (0xE000..=0xFDFF).contains(&loc) {
                    loc -= 0x2000;
                }
                if (0x8000..0xA000).contains(&loc) {
                    let lcdc = self.io_regs[0x40];
                    if lcdc & 0x80 != 0 && self.io_regs[0x41] & 0x03 == 3 {
                        return 0xFF;
                    }
                    if self.cgb_mode && self.vram_bank_sel & 1 == 1 {
                        return self.vram_bank1[loc as usize - 0x8000];
                    }
                    return self.vram[loc as usize - 0x8000];
                }
                if self.cgb_mode && (0xD000..0xE000).contains(&loc) {
                    return self.wram_banks
                        [self.wram_bank as usize * 0x1000 + loc as usize - 0xD000];
                }
                if (0xFE00..0xFEA0).contains(&loc) {
                    let lcdc = self.io_regs[0x40];
                    if lcdc & 0x80 != 0 {
                        let mode = self.io_regs[0x41] & 0x03;
                        if mode == 2 || mode == 3 {
                            return 0xFF;
                        }
                    }
                    return self.oam[loc as usize - 0xFE00];
                }
                match loc {
                    0xFF4F if self.cgb_mode => (self.vram_bank_sel & 1) | 0xFE,
                    0xFF70 if self.cgb_mode => self.wram_bank | 0xF8,
                    0xFF00 => {
                        let sel = self.io_regs[0x00];
                        let mut result = (sel & 0x30) | 0xCF;
                        if sel & 0x10 == 0 {
                            if self.joypad_dirs & 0x01 != 0 {
                                result &= 0xFE;
                            }
                            if self.joypad_dirs & 0x02 != 0 {
                                result &= 0xFD;
                            }
                            if self.joypad_dirs & 0x04 != 0 {
                                result &= 0xFB;
                            }
                            if self.joypad_dirs & 0x08 != 0 {
                                result &= 0xF7;
                            }
                        }
                        if sel & 0x20 == 0 {
                            if self.joypad_btns & 0x01 != 0 {
                                result &= 0xFE;
                            }
                            if self.joypad_btns & 0x02 != 0 {
                                result &= 0xFD;
                            }
                            if self.joypad_btns & 0x04 != 0 {
                                result &= 0xFB;
                            }
                            if self.joypad_btns & 0x08 != 0 {
                                result &= 0xF7;
                            }
                        }
                        result
                    }
                    0xC000..0xD000 => self.wram[(loc - 0xC000) as usize],
                    0xD000..0xE000 => self.wram_banks[(loc - 0xD000) as usize],
                    0xFEA0..0xFF00 => 0xFF,
                    0xFF80..=0xFFFF => self.hram[(loc - 0xFF80) as usize],
                    loc if loc >= 0xFF00 => self.io_regs[(loc - 0xFF00) as usize],
                    _ => 0xFF,
                }
            }
        }
    }

    pub fn r_mem_raw(&self, loc: u16) -> u8 {
        match loc {
            ..0x8000 => match self.cart_type {
                0x00 => self.no_mbc_read_rom(loc),
                0x01 | 0x02 | 0x03 => self.mbc1_read_rom(loc),
                0x05 | 0x06 => self.mbc2_read_rom(loc),
                0x0F | 0x10 | 0x11 | 0x12 | 0x13 => self.mbc3_read_rom(loc),
                0x19 | 0x1A | 0x1B | 0x1C | 0x1D | 0x1E => self.mbc5_read_rom(loc),
                _ => 0xFF,
            },
            0xA000..0xC000 => self.extern_ram[(loc - 0xA000) as usize],
            loc => self.region_read_raw(loc),
        }
    }

    pub fn r_ram_raw(&self, loc: u16) -> u8 {
        self.extern_ram[loc as usize]
    }

    pub fn w_mem(&mut self, loc: u16, val: u8) {
        match loc {
            ..0x8000 => match self.cart_type {
                0x00 => self.no_mbc_write_rom(loc, val),
                0x01 | 0x02 | 0x03 => self.mbc1_write_rom(loc, val),
                0x05 | 0x06 => self.mbc2_write_rom(loc, val),
                0x0F | 0x10 | 0x11 | 0x12 | 0x13 => self.mbc3_write_rom(loc, val),
                0x19 | 0x1A | 0x1B | 0x1C | 0x1D | 0x1E => self.mbc5_write_rom(loc, val),
                _ => (),
            },
            0xA000..0xC000 => match self.cart_type {
                0x00 => self.no_mbc_write_ram(loc, val),
                0x01 | 0x02 | 0x03 => self.mbc1_write_ram(loc, val),
                0x05 | 0x06 => self.mbc2_write_ram(loc, val),
                0x0F | 0x10 | 0x11 | 0x12 | 0x13 => self.mbc3_write_ram(loc, val),
                0x19 | 0x1A | 0x1B | 0x1C | 0x1D | 0x1E => self.mbc5_write_ram(loc, val),
                _ => (),
            },
            mut loc => {
                if (0xE000..=0xFDFF).contains(&loc) {
                    loc -= 0x2000;
                }
                if (0x8000..0xA000).contains(&loc) {
                    let lcdc = self.io_regs[0x40];
                    if lcdc & 0x80 != 0 && self.io_regs[0x41] & 0x03 == 3 {
                        return;
                    }
                    if self.cgb_mode && self.vram_bank_sel & 1 == 1 {
                        self.vram_bank1[loc as usize - 0x8000] = val;
                        return;
                    }
                    self.vram[loc as usize - 0x8000] = val;
                    return;
                }
                if self.cgb_mode && (0xD000..0xE000).contains(&loc) {
                    self.wram_banks[self.wram_bank as usize * 0x1000 + loc as usize - 0xD000] = val;
                    return;
                }
                if (0xFE00..0xFEA0).contains(&loc) {
                    let lcdc = self.io_regs[0x40];
                    if lcdc & 0x80 != 0 {
                        let mode = self.io_regs[0x41] & 0x03;
                        if mode == 2 || mode == 3 {
                            return;
                        }
                    }
                    self.oam[loc as usize - 0xFE00] = val;
                    return;
                }
                if loc == 0xFF04 {
                    self.io_regs[0x4E] = self.io_regs[0x04];
                    self.div_reset_pending = true;
                    self.io_regs[0x04] = 0x00;
                } else if loc == 0xFF02 && val & 0x81 == 0x81 {
                    self.io_regs[(loc - 0xFF00) as usize] = val & 0x7F;
                    self.io_regs[0x0F] |= 0x08;
                    self.serial_byte_pending = true;
                } else if self.cgb_mode && loc == 0xFF4F {
                    self.vram_bank_sel = val & 1;
                } else if self.cgb_mode && loc == 0xFF55 {
                    if self.hdma_active && val & 0x80 == 0 {
                        self.hdma_active = false;
                        self.io_regs[0x55] = 0x80 | self.hdma_remaining;
                    } else if val & 0x80 == 0 {
                        let src =
                            ((self.io_regs[0x51] as u16) << 8) | (self.io_regs[0x52] as u16 & 0xF0);
                        let dst = 0x8000u16
                            | ((self.io_regs[0x53] as u16 & 0x1F) << 8)
                            | (self.io_regs[0x54] as u16 & 0xF0);
                        let blocks = (val & 0x7F) as u16 + 1;
                        for i in 0..blocks * 0x10 {
                            let byte = self.r_mem(src.wrapping_add(i));
                            let addr = dst.wrapping_add(i);
                            if self.vram_bank_sel & 1 == 0 {
                                self.vram[(addr - 0x8000) as usize] = byte;
                            } else {
                                self.vram_bank1[(addr - 0x8000) as usize] = byte;
                            }
                        }
                        self.io_regs[0x55] = 0xFF;
                    } else {
                        self.hdma_src =
                            ((self.io_regs[0x51] as u16) << 8) | (self.io_regs[0x52] as u16 & 0xF0);
                        self.hdma_dst = 0x8000u16
                            | ((self.io_regs[0x53] as u16 & 0x1F) << 8)
                            | (self.io_regs[0x54] as u16 & 0xF0);
                        self.hdma_remaining = val & 0x7F;
                        self.hdma_active = true;
                        self.io_regs[0x55] = val & 0x7F;
                    }
                } else if self.cgb_mode && loc == 0xFF70 {
                    self.wram_bank = if val & 7 == 0 { 1 } else { val & 7 };
                    self.io_regs[0x70] = self.wram_bank;
                } else {
                    match loc {
                        0xC000..0xD000 => self.wram[(loc - 0xC000) as usize] = val,
                        0xD000..0xE000 => self.wram_banks[(loc - 0xD000) as usize] = val,
                        0xFF80..=0xFFFF => self.hram[(loc - 0xFF80) as usize] = val,
                        _ => {
                            if loc >= 0xFF00 {
                                self.io_regs[(loc - 0xFF00) as usize] = val;
                            }
                        }
                    }
                }
            }
        }
    }

    pub fn w_mem_raw(&mut self, loc: u16, val: u8) {
        match loc {
            0xA000..0xC000 => self.extern_ram[(loc - 0xA000) as usize] = val,
            loc => self.region_write_raw(loc, val),
        }
    }

    pub fn w_ram_raw(&mut self, loc: u16, val: u8) {
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

    pub fn take_div_reset(&mut self) -> bool {
        let pending = self.div_reset_pending;
        self.div_reset_pending = false;
        pending
    }

    pub fn take_serial_byte(&mut self) -> Option<u8> {
        if self.serial_byte_pending {
            self.serial_byte_pending = false;
            Some(self.io_regs[0x01])
        } else {
            None
        }
    }

    pub fn get_vram_bank1_byte(&self, addr: u16) -> u8 {
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
        self.io_regs[0x70] = self.wram_bank;
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
        self.io_regs[0x50] == 0
    }

    pub fn disable_boot(&mut self) {
        self.io_regs[0x50] = 1;
    }

    pub fn do_hdma_block(&mut self) {
        if !self.hdma_active {
            return;
        }
        for i in 0..0x10u16 {
            let byte = self.r_mem(self.hdma_src.wrapping_add(i));
            let addr = self.hdma_dst.wrapping_add(i);
            if self.vram_bank_sel & 1 == 0 {
                self.vram[(addr - 0x8000) as usize] = byte;
            } else {
                self.vram_bank1[(addr - 0x8000) as usize] = byte;
            }
        }
        self.hdma_src = self.hdma_src.wrapping_add(0x10);
        self.hdma_dst = 0x8000 | (self.hdma_dst.wrapping_add(0x10) & 0x1FFF);
        if self.hdma_remaining == 0 {
            self.hdma_active = false;
            self.io_regs[0x55] = 0xFF;
        } else {
            self.hdma_remaining -= 1;
            self.io_regs[0x55] = self.hdma_remaining;
        }
    }

    pub fn read_io(&self, offset: u8) -> u8 {
        self.io_regs[offset as usize]
    }

    pub fn write_io(&mut self, offset: u8, val: u8) {
        self.io_regs[offset as usize] = val;
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

    pub fn set_joypad(&mut self, btns: u8, dirs: u8) {
        let new_press = (!self.joypad_btns & btns) | (!self.joypad_dirs & dirs);
        self.joypad_btns = btns;
        self.joypad_dirs = dirs;
        if new_press != 0 {
            self.io_regs[0x0F] |= 0x10;
        }
    }

    fn post_boot_init(&mut self) {
        self.io_regs[0x50] = 1;
        self.io_regs[0x01] = 0x00;
        self.io_regs[0x05] = 0x00;
        self.io_regs[0x06] = 0x00;
        self.io_regs[0x07] = 0xF8;
        self.io_regs[0x0F] = 0xE1;
        self.io_regs[0x10] = 0x80;
        self.io_regs[0x11] = 0xBF;
        self.io_regs[0x12] = 0xF3;
        self.io_regs[0x13] = 0xFF;
        self.io_regs[0x14] = 0xBF;
        self.io_regs[0x16] = 0x3F;
        self.io_regs[0x17] = 0x00;
        self.io_regs[0x18] = 0xFF;
        self.io_regs[0x19] = 0xBF;
        self.io_regs[0x1A] = 0x7F;
        self.io_regs[0x1B] = 0xFF;
        self.io_regs[0x1C] = 0x9F;
        self.io_regs[0x1D] = 0xFF;
        self.io_regs[0x1E] = 0xBF;
        self.io_regs[0x20] = 0xFF;
        self.io_regs[0x21] = 0x00;
        self.io_regs[0x22] = 0x00;
        self.io_regs[0x23] = 0xBF;
        self.io_regs[0x24] = 0x77;
        self.io_regs[0x25] = 0xF3;
        self.io_regs[0x26] = 0xF1;
        self.hram[0x7F] = 0x00;
        if self.cgb_mode {
            self.io_regs[0x00] = 0xCF;
            self.io_regs[0x02] = 0x7F;
            self.io_regs[0x46] = 0x00;
            self.io_regs[0x4D] = 0x7E;
            self.io_regs[0x51] = 0xFF;
            self.io_regs[0x52] = 0xFF;
            self.io_regs[0x53] = 0xFF;
            self.io_regs[0x54] = 0xFF;
            self.io_regs[0x55] = 0xFF;
            self.io_regs[0x56] = 0x3E;
            self.io_regs[0x70] = 0x01;
        } else {
            self.io_regs[0x00] = 0xCF;
            self.io_regs[0x02] = 0x7E;
            self.io_regs[0x04] = 0xAB;
            self.io_regs[0x46] = 0xFF;
        }
    }
}
