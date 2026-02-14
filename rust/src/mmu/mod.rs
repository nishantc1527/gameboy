mod io;
mod mbc1;
mod mbc3;
mod no_mbc;

use crate::log_err;
use std::{fs::File, io::Read, path::Path};

#[repr(u8)]
enum TestCategory {
    Blargg = 0,
    Mooneye = 1,
}

pub struct MMU {
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
    test_category: u8,
}

impl MMU {
    pub fn new(rom_file_name: &str, boot_rom_file_name: &str, test_category: u8) -> Option<MMU> {
        let mut rom_title = String::new();
        let cart_type: u8;
        let rom_size: u8;
        let ram_size: u8;
        let mem = vec![0u8; 0x800000];
        let mut brom = vec![0u8; 0x100];
        let mut rom = vec![0u8; 0x800000];
        let extern_ram = vec![0u8; 0x20000];
        let rom_bank: u8;
        let ram_bank: u8 = 0;
        let ram_enable;
        let mut mbc1_1mb_mode = false;

        // log_info!("OPENING BOOT ROM FILE\n");
        let mut boot_rom_file = File::open(Path::new(boot_rom_file_name)).ok()?;
        boot_rom_file.read(&mut brom).ok()?;
        // log_info!("OPENING ROM FILE\n");
        let mut rom_file = File::open(Path::new(rom_file_name)).ok()?;
        rom_file.read(&mut rom).ok()?;
        // log_info!("SUCCESSFULLY READ FILES\n");

        for i in 0x0134u16..=0x0142u16 {
            rom_title.push(rom[i as usize] as char);
        }
        rom_title.push('\0');
        cart_type = rom[0x0147];
        rom_size = rom[0x0148];
        ram_size = rom[0x0149];
        match cart_type {
            0x00 | 0x01 | 0x03 | 0x11 | 0x13 => (),
            _ => {
                log_err!("UNIMPLEMENTED MAPPER ${:02X}\n", cart_type);
                return None;
            }
        }
        // log_info!("USING MAPPER: ${:02X}\n", cart_type);
        match rom_size {
            0x00 | 0x01 | 0x03 | 0x04 | 0x05 | 0x07 => (),
            _ => {
                log_err!("UNIMPLEMENTED ROM SIZE: ${:02X}\n", rom_size);
                return None;
            }
        }
        // log_info!("USING ROM SIZE: ${:02X}", rom_size);
        match ram_size {
            0x00 | 0x02 | 0x03 => (),
            _ => {
                log_err!("UNIMPLEMENTED RAM SIZE: ${:02X}\n", ram_size);
                return None;
            }
        }
        // log_info!("USING RAM SIZE: ${:02X}\n", ram_size);
        rom_bank = 1;
        ram_enable = false;

        match cart_type {
            0x01 | 0x03 => {
                mbc1_1mb_mode = false;
                if rom_size > 0x06 {
                    log_err!("ROM SIZE NOT AVAILABLE\n");
                    return None;
                }
                if ram_size > 0x03 {
                    log_err!("RAM SIZE NOT AVAILABLE\n");
                    return None;
                }
            }
            0x13 => {
                if rom_size > 0x06 {
                    log_err!("ROM SIZE NOT AVAILABLE\n");
                    return None;
                }
                if ram_size > 0x03 {
                    log_err!("RAM SIZE NOT AVAILABE\n");
                    return None;
                }
            }
            _ => (),
        }
        Some(MMU {
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

    pub fn r_mem(&self, loc: u16) -> u8 {
        if self.mem[0xFF50] == 0 && loc < 0x100 {
            return self.brom[loc as usize];
        }
        match loc {
            ..0x8000 => match self.cart_type {
                0x00 => self.no_mbc_read_rom(loc),
                0x01 | 0x03 => self.mbc1_read_rom(loc),
                0x13 => self.mbc3_read_rom(loc),
                _ => 0xFF,
            },
            0xA000..0xC000 => match self.cart_type {
                0x00 => self.no_mbc_read_ram(loc),
                0x01 | 0x03 => self.mbc1_read_ram(loc),
                0x13 => self.mbc3_read_ram(loc),
                _ => 0xFF,
            },
            mut loc => {
                if loc >= 0xE000 && loc <= 0xFDFF {
                    loc -= 0x2000;
                }
                self.mem[loc as usize]
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
                0x01 | 0x03 => self.mbc1_write_rom(loc, val),
                0x13 => self.mbc3_write_rom(loc, val),
                _ => (),
            },
            0xA000..0xC000 => match self.cart_type {
                0x00 => self.no_mbc_write_ram(loc, val),
                0x01 | 0x03 => self.mbc1_write_ram(loc, val),
                0x13 => self.mbc3_write_ram(loc, val),
                _ => (),
            },
            mut loc => {
                if loc >= 0xE000 && loc <= 0xFDFF {
                    loc -= 0x2000;
                }
                if self.test_category == TestCategory::Blargg as u8 && loc == 0xFF01 {
                    print!("{}", val as char);
                }
                if loc == 0xFF04 {
                    self.mem[loc as usize] = 0x00;
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
