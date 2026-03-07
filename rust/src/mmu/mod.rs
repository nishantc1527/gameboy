mod apu_reg;
mod cpu_reg;
mod io;
mod mbc1;
mod mbc2;
mod mbc3;
mod mbc5;
mod no_mbc;
mod ppu_reg;

use super::TestCategory;
use std::{fs::File, io::Read, path::Path};

pub struct Mmu {
    rom_title: String,
    cart_type: u8,
    rom_size: u8,
    ram_size: u8,
    mem: Vec<u8>,
    brom: Vec<u8>,
    rom: Vec<u8>,
    extern_ram: Vec<u8>,
    rom_bank: u8,
    rom_bank_hi: u8,
    ram_bank: u8,
    ram_enable: bool,
    mbc1_1mb_mode: bool,
    mbc1_multicart: bool,
    test_category: i8,
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
    div_reset_pending: bool,
    vram_bank1: Vec<u8>,
    vram_bank_sel: u8,
    wram_banks: Vec<u8>,
    wram_bank: u8,
    bg_pal_ram: [u8; 64],
    obj_pal_ram: [u8; 64],
    boot_skipped: bool,
}

#[allow(clippy::manual_range_patterns)]
impl Mmu {
    pub fn new(rom_file_name: &str, boot_rom_file_name: &str, test_category: i8) -> Option<Mmu> {
        let mut rom_title = String::new();
        let mem = vec![0u8; 0x800000];
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
        let cgb_mode = matches!(rom[0x0143], 0x80 | 0xC0);
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
        for i in 0x0134usize..=0x014C {
            checksum = checksum.wrapping_sub(rom[i]).wrapping_sub(1);
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
        // println!("USING MAPPER: ${:02X}\n", cart_type);
        match rom_size {
            0x00 | 0x01 | 0x02 | 0x03 | 0x04 | 0x05 | 0x06 | 0x07 | 0x08 => (),
            _ => {
                eprintln!("UNIMPLEMENTED ROM SIZE: ${:02X}\n", rom_size);
                return None;
            }
        }
        // println!("USING ROM SIZE: ${:02X}", rom_size);
        match ram_size {
            0x00 | 0x02 | 0x03 | 0x04 => (),
            _ => {
                eprintln!("UNIMPLEMENTED RAM SIZE: ${:02X}\n", ram_size);
                return None;
            }
        }
        // println!("USING RAM SIZE: ${:02X}\n", ram_size);
        let rom_bank: u8 = 1;
        let rom_bank_hi: u8 = 0;
        let ram_enable = false;

        match cart_type {
            0x01 | 0x02 | 0x03 => {
                mbc1_1mb_mode = false;
                mbc1_multicart = mbc1::detect_multicart(&rom, rom_size);
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
            mem,
            brom,
            rom,
            extern_ram,
            rom_bank,
            rom_bank_hi,
            ram_bank,
            ram_enable,
            mbc1_1mb_mode,
            mbc1_multicart,
            test_category,
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
            div_reset_pending: false,
            vram_bank1: vec![0u8; 0x2000],
            vram_bank_sel: 0,
            wram_banks: vec![0u8; 0x8000],
            wram_bank: 1,
            bg_pal_ram: [0u8; 64],
            obj_pal_ram: [0u8; 64],
            boot_skipped,
        };
        if cgb_mode {
            mmu.mem[0xFF4D] = 0;
            for i in 0..32usize {
                mmu.bg_pal_ram[i * 2] = 0xFF;
                mmu.bg_pal_ram[i * 2 + 1] = 0x7F;
            }
        }
        if boot_skipped {
            mmu.post_boot_init();
        }
        Some(mmu)
    }

    #[allow(clippy::identity_op)]
    pub fn r_mem(&self, loc: u16) -> u8 {
        if self.mem[0xFF50] == 0 {
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
                    let lcdc = self.mem[0xFF40];
                    if lcdc & 0x80 != 0 && self.mem[0xFF41] & 0x03 == 3 {
                        return 0xFF;
                    }
                    if self.cgb_mode && self.vram_bank_sel & 1 == 1 {
                        return self.vram_bank1[loc as usize - 0x8000];
                    }
                }
                if self.cgb_mode && (0xD000..0xE000).contains(&loc) {
                    return self.wram_banks
                        [self.wram_bank as usize * 0x1000 + loc as usize - 0xD000];
                }
                if (0xFE00..0xFEA0).contains(&loc) {
                    let lcdc = self.mem[0xFF40];
                    if lcdc & 0x80 != 0 {
                        let mode = self.mem[0xFF41] & 0x03;
                        if mode == 2 || mode == 3 {
                            return 0xFF;
                        }
                    }
                }
                match loc {
                    0xFF4F if self.cgb_mode => (self.vram_bank_sel & 1) | 0xFE,
                    0xFF69 if self.cgb_mode => self.bg_pal_ram[(self.mem[0xFF68] & 0x3F) as usize],
                    0xFF6B if self.cgb_mode => self.obj_pal_ram[(self.mem[0xFF6A] & 0x3F) as usize],
                    0xFF70 if self.cgb_mode => self.wram_bank | 0xF8,
                    apu_reg::NR10 => self.mem[loc as usize] | 0x80,
                    apu_reg::NR11 => self.mem[loc as usize] | 0x3F,
                    apu_reg::NR12 => self.mem[loc as usize] | 0x00,
                    apu_reg::NR13 => self.mem[loc as usize] | 0xFF,
                    apu_reg::NR14 => self.mem[loc as usize] | 0xBF,
                    0xFF15 => 0xFF,
                    apu_reg::NR21 => self.mem[loc as usize] | 0x3F,
                    apu_reg::NR22 => self.mem[loc as usize] | 0x00,
                    apu_reg::NR23 => self.mem[loc as usize] | 0xFF,
                    apu_reg::NR24 => self.mem[loc as usize] | 0xBF,
                    apu_reg::NR30 => self.mem[loc as usize] | 0x7F,
                    apu_reg::NR31 => self.mem[loc as usize] | 0xFF,
                    apu_reg::NR32 => self.mem[loc as usize] | 0x9F,
                    apu_reg::NR33 => self.mem[loc as usize] | 0xFF,
                    apu_reg::NR34 => self.mem[loc as usize] | 0xBF,
                    0xFF1F => 0xFF,
                    apu_reg::NR41 => self.mem[loc as usize] | 0xFF,
                    apu_reg::NR42 => self.mem[loc as usize] | 0x00,
                    apu_reg::NR43 => self.mem[loc as usize] | 0x00,
                    apu_reg::NR44 => self.mem[loc as usize] | 0xBF,
                    apu_reg::NR50 => self.mem[loc as usize] | 0x00,
                    apu_reg::NR51 => self.mem[loc as usize] | 0x00,
                    apu_reg::NR52 => self.mem[loc as usize] | 0x70,
                    0xFF27..0xFF30 => 0xFF,
                    0xFF00 => {
                        let sel = self.mem[0xFF00];
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
                    loc => self.mem[loc as usize],
                }
            }
        }
    }

    pub fn r_mem_raw(&self, loc: u16) -> u8 {
        self.mem[loc as usize]
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
                    let lcdc = self.mem[0xFF40];
                    if lcdc & 0x80 != 0 && self.mem[0xFF41] & 0x03 == 3 {
                        return;
                    }
                    if self.cgb_mode && self.vram_bank_sel & 1 == 1 {
                        self.vram_bank1[loc as usize - 0x8000] = val;
                        return;
                    }
                }
                if self.cgb_mode && (0xD000..0xE000).contains(&loc) {
                    self.wram_banks[self.wram_bank as usize * 0x1000 + loc as usize - 0xD000] = val;
                    return;
                }
                if (0xFE00..0xFEA0).contains(&loc) {
                    let lcdc = self.mem[0xFF40];
                    if lcdc & 0x80 != 0 {
                        let mode = self.mem[0xFF41] & 0x03;
                        if mode == 2 || mode == 3 {
                            return;
                        }
                    }
                }
                if (self.test_category == TestCategory::TestBlarggCpu as i8
                    || self.test_category == TestCategory::TestBlarggAudio as i8
                    || self.test_category == TestCategory::TestBlarggCpuTime as i8
                    || self.test_category == TestCategory::TestBlarggMemTime as i8)
                    && loc == 0xFF01
                {
                    print!("{}", val as char);
                }
                if (0xFF10..0xFF26).contains(&loc) && loc != 0xFF20 && self.mem[0xFF26] & 0x80 == 0
                {
                    if loc == 0xFF11 || loc == 0xFF16 {
                        self.mem[loc as usize] = val & 0x3F;
                        let bit: u8 = if loc == 0xFF11 { 0x01 } else { 0x02 };
                        self.mem[0xFF4C] |= bit;
                    } else if loc == 0xFF1B {
                        self.mem[loc as usize] = val;
                        self.mem[0xFF4C] |= 0x04;
                    }
                } else if loc == 0xFF04 {
                    self.mem[0xFF4E] = self.mem[0xFF04];
                    self.div_reset_pending = true;
                    self.mem[loc as usize] = 0x00;
                } else if loc == 0xFF02 && val & 0x81 == 0x81 {
                    self.mem[loc as usize] = val & 0x7F;
                    self.mem[0xFF0F] |= 0x08;
                } else if self.cgb_mode && loc == 0xFF4F {
                    self.vram_bank_sel = val & 1;
                } else if self.cgb_mode && loc == 0xFF68 {
                    self.mem[0xFF68] = val;
                } else if self.cgb_mode && loc == 0xFF69 {
                    let idx = (self.mem[0xFF68] & 0x3F) as usize;
                    self.bg_pal_ram[idx] = val;
                    if self.mem[0xFF68] & 0x80 != 0 {
                        self.mem[0xFF68] = (self.mem[0xFF68] & 0x80) | ((idx as u8 + 1) & 0x3F);
                    }
                } else if self.cgb_mode && loc == 0xFF6A {
                    self.mem[0xFF6A] = val;
                } else if self.cgb_mode && loc == 0xFF6B {
                    let idx = (self.mem[0xFF6A] & 0x3F) as usize;
                    self.obj_pal_ram[idx] = val;
                    if self.mem[0xFF6A] & 0x80 != 0 {
                        self.mem[0xFF6A] = (self.mem[0xFF6A] & 0x80) | ((idx as u8 + 1) & 0x3F);
                    }
                } else if self.cgb_mode && loc == 0xFF70 {
                    self.wram_bank = if val & 7 == 0 { 1 } else { val & 7 };
                    self.mem[0xFF70] = self.wram_bank;
                } else {
                    match loc {
                        0xFF11 => self.mem[0xFF4C] |= 0x01,
                        0xFF16 => self.mem[0xFF4C] |= 0x02,
                        0xFF1B => self.mem[0xFF4C] |= 0x04,
                        0xFF20 => self.mem[0xFF4C] |= 0x08,
                        _ => {}
                    }
                    self.mem[loc as usize] = val;
                }
            }
        }
    }

    pub fn w_mem_raw(&mut self, loc: u16, val: u8) {
        self.mem[loc as usize] = val;
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

    pub fn take_div_reset(&mut self) -> bool {
        let pending = self.div_reset_pending;
        self.div_reset_pending = false;
        pending
    }

    pub fn get_vram_bank1_byte(&self, addr: u16) -> u8 {
        self.vram_bank1[(addr - 0x8000) as usize]
    }

    pub fn get_bg_pal_byte(&self, idx: u8) -> u8 {
        self.bg_pal_ram[idx as usize]
    }

    pub fn get_obj_pal_byte(&self, idx: u8) -> u8 {
        self.obj_pal_ram[idx as usize]
    }

    pub fn boot_skipped(&self) -> bool {
        self.boot_skipped
    }

    fn post_boot_init(&mut self) {
        self.mem[0xFF50] = 1;
        self.mem[0xFF01] = 0x00;
        self.mem[0xFF05] = 0x00;
        self.mem[0xFF06] = 0x00;
        self.mem[0xFF07] = 0xF8;
        self.mem[0xFF0F] = 0xE1;
        self.mem[0xFF10] = 0x80;
        self.mem[0xFF11] = 0xBF;
        self.mem[0xFF12] = 0xF3;
        self.mem[0xFF13] = 0xFF;
        self.mem[0xFF14] = 0xBF;
        self.mem[0xFF16] = 0x3F;
        self.mem[0xFF17] = 0x00;
        self.mem[0xFF18] = 0xFF;
        self.mem[0xFF19] = 0xBF;
        self.mem[0xFF1A] = 0x7F;
        self.mem[0xFF1B] = 0xFF;
        self.mem[0xFF1C] = 0x9F;
        self.mem[0xFF1D] = 0xFF;
        self.mem[0xFF1E] = 0xBF;
        self.mem[0xFF20] = 0xFF;
        self.mem[0xFF21] = 0x00;
        self.mem[0xFF22] = 0x00;
        self.mem[0xFF23] = 0xBF;
        self.mem[0xFF24] = 0x77;
        self.mem[0xFF25] = 0xF3;
        self.mem[0xFF26] = 0xF1;
        self.mem[0xFF40] = 0x91;
        self.mem[0xFF42] = 0x00;
        self.mem[0xFF43] = 0x00;
        self.mem[0xFF44] = 0x00;
        self.mem[0xFF45] = 0x00;
        self.mem[0xFF47] = 0xFC;
        self.mem[0xFF4A] = 0x00;
        self.mem[0xFF4B] = 0x00;
        self.mem[0xFFFF] = 0x00;
        if self.cgb_mode {
            self.mem[0xFF00] = 0xCF;
            self.mem[0xFF02] = 0x7F;
            self.mem[0xFF41] = 0x85;
            self.mem[0xFF46] = 0x00;
            self.mem[0xFF4D] = 0x7E;
            self.mem[0xFF51] = 0xFF;
            self.mem[0xFF52] = 0xFF;
            self.mem[0xFF53] = 0xFF;
            self.mem[0xFF54] = 0xFF;
            self.mem[0xFF55] = 0xFF;
            self.mem[0xFF56] = 0x3E;
            self.mem[0xFF70] = 0x01;
        } else {
            self.mem[0xFF00] = 0xCF;
            self.mem[0xFF02] = 0x7E;
            self.mem[0xFF04] = 0xAB;
            self.mem[0xFF41] = 0x85;
            self.mem[0xFF46] = 0xFF;
        }
    }
}
