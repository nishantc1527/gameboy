use super::Mmu;

impl Mmu {
    pub fn mbc5_read_rom(&self, loc: u16) -> u8 {
        match loc {
            ..0x4000 => self.rom[loc as usize],
            loc => {
                let bank = ((self.rom_bank_hi as usize) << 8) | (self.rom_bank as usize);
                let num_banks = 1usize << (self.rom_size as usize + 1);
                let effective_bank = bank & (num_banks - 1);
                self.rom[effective_bank * 0x4000 + (loc as usize - 0x4000)]
            }
        }
    }

    pub fn mbc5_read_ram(&self, loc: u16) -> u8 {
        if self.ram_enable {
            return match self.ram_size {
                0x00 => 0xFF,
                0x02 => self.extern_ram[(loc - 0xA000) as usize],
                0x03 => self.extern_ram[(loc - 0xA000) as usize + 0x2000 * self.ram_bank as usize],
                0x04 => self.extern_ram[(loc - 0xA000) as usize + 0x2000 * self.ram_bank as usize],
                _ => 0xFF,
            };
        }
        0xFF
    }

    pub fn mbc5_write_rom(&mut self, loc: u16, val: u8) {
        match loc {
            ..0x2000 => self.ram_enable = (val & 0x0F) == 0x0A,
            0x2000..0x3000 => self.rom_bank = val,
            0x3000..0x4000 => self.rom_bank_hi = val & 0x01,
            0x4000..0x6000 => self.ram_bank = val & 0x0F,
            _ => (),
        }
    }

    pub fn mbc5_write_ram(&mut self, loc: u16, val: u8) {
        if self.ram_enable {
            match self.ram_size {
                0x00 => (),
                0x02 => self.extern_ram[(loc - 0xA000) as usize] = val,
                0x03 => {
                    self.extern_ram[(loc - 0xA000) as usize + 0x2000 * self.ram_bank as usize] = val
                }
                0x04 => {
                    self.extern_ram[(loc - 0xA000) as usize + 0x2000 * self.ram_bank as usize] = val
                }
                _ => (),
            }
        }
    }
}
