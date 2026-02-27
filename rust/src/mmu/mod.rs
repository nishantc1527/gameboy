mod apu_reg;
mod cpu_reg;
mod io;
mod mbc1;
mod mbc3;
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
    ram_bank: u8,
    ram_enable: bool,
    mbc1_1mb_mode: bool,
    test_category: i8,
}

#[allow(clippy::manual_range_patterns)]
impl Mmu {
    pub fn new(rom_file_name: &str, boot_rom_file_name: &str, test_category: i8) -> Option<Mmu> {
        let mut rom_title = String::new();
        let mem = vec![0u8; 0x800000];
        let mut brom = vec![0u8; 0x100];
        let mut rom = vec![0u8; 0x800000];
        let extern_ram = vec![0u8; 0x20000];
        let ram_bank: u8 = 0;
        let mut mbc1_1mb_mode = false;
        // println!("OPENING BOOT ROM FILE\n");
        let mut boot_rom_file = File::open(Path::new(boot_rom_file_name)).ok()?;
        if boot_rom_file.read(&mut brom).ok()? != 0x100 {
            eprintln!("COULD NOT READ FULL BOOT ROM");
            return None;
        }
        // println!("OPENING ROM FILE\n");
        let mut rom_file = File::open(Path::new(rom_file_name)).ok()?;
        let _ = rom_file.read(&mut rom).ok()?;
        // println!("SUCCESSFULLY READ FILES\n");

        (0x0134usize..=0x0142usize).for_each(|i| {
            rom_title.push(rom[i] as char);
        });
        rom_title.push('\0');
        let cart_type: u8 = rom[0x0147];
        let rom_size: u8 = rom[0x0148];
        let ram_size: u8 = rom[0x0149];
        match cart_type {
            0x00 | 0x01 | 0x02 | 0x03 | 0x11 | 0x13 => (),
            _ => {
                eprintln!("UNIMPLEMENTED MAPPER ${:02X}\n", cart_type);
                return None;
            }
        }
        // println!("USING MAPPER: ${:02X}\n", cart_type);
        match rom_size {
            0x00 | 0x01 | 0x03 | 0x04 | 0x05 | 0x07 => (),
            _ => {
                eprintln!("UNIMPLEMENTED ROM SIZE: ${:02X}\n", rom_size);
                return None;
            }
        }
        // println!("USING ROM SIZE: ${:02X}", rom_size);
        match ram_size {
            0x00 | 0x02 | 0x03 => (),
            _ => {
                eprintln!("UNIMPLEMENTED RAM SIZE: ${:02X}\n", ram_size);
                return None;
            }
        }
        // println!("USING RAM SIZE: ${:02X}\n", ram_size);
        let rom_bank: u8 = 1;
        let ram_enable = false;

        match cart_type {
            0x01 | 0x02 | 0x03 => {
                mbc1_1mb_mode = false;
                if rom_size > 0x06 {
                    eprintln!("ROM SIZE NOT AVAILABLE\n");
                    return None;
                }
                if ram_size > 0x03 {
                    eprintln!("RAM SIZE NOT AVAILABLE\n");
                    return None;
                }
            }
            0x11 | 0x12 | 0x13 => {
                if rom_size > 0x07 {
                    eprintln!("ROM SIZE NOT AVAILABLE\n");
                    return None;
                }
                if ram_size > 0x03 {
                    eprintln!("RAM SIZE NOT AVAILABLE\n");
                    return None;
                }
            }
            _ => (),
        }
        Some(Mmu {
            rom_title,
            cart_type,
            rom_size,
            ram_size,
            mem,
            brom,
            rom,
            extern_ram,
            rom_bank,
            ram_bank,
            ram_enable,
            mbc1_1mb_mode,
            test_category,
        })
    }

    #[allow(clippy::identity_op)]
    pub fn r_mem(&self, loc: u16) -> u8 {
        if self.mem[0xFF50] == 0 && loc < 0x100 {
            return self.brom[loc as usize];
        }
        match loc {
            ..0x8000 => match self.cart_type {
                0x00 => self.no_mbc_read_rom(loc),
                0x01 | 0x02 | 0x03 => self.mbc1_read_rom(loc),
                0x11 | 0x12 | 0x13 => self.mbc3_read_rom(loc),
                _ => 0xFF,
            },
            0xA000..0xC000 => match self.cart_type {
                0x00 => self.no_mbc_read_ram(loc),
                0x01 | 0x02 | 0x03 => self.mbc1_read_ram(loc),
                0x11 | 0x12 | 0x13 => self.mbc3_read_ram(loc),
                _ => 0xFF,
            },
            mut loc => {
                if (0xE000..=0xFDFF).contains(&loc) {
                    loc -= 0x2000;
                }
                match loc {
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
                0x11 | 0x12 | 0x13 => self.mbc3_write_rom(loc, val),
                _ => (),
            },
            0xA000..0xC000 => match self.cart_type {
                0x00 => self.no_mbc_write_ram(loc, val),
                0x01 | 0x02 | 0x03 => self.mbc1_write_ram(loc, val),
                0x11 | 0x12 | 0x13 => self.mbc3_write_ram(loc, val),
                _ => (),
            },
            mut loc => {
                if (0xE000..=0xFDFF).contains(&loc) {
                    loc -= 0x2000;
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
                } else if loc == 0xFF04 {
                    self.mem[loc as usize] = 0x00;
                } else if loc == 0xFF02 && val & 0x81 == 0x81 {
                    self.mem[loc as usize] = val & 0x7F;
                    self.mem[0xFF0F] |= 0x08;
                } else {
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
}
