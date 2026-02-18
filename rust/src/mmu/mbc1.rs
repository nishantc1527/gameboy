use super::Mmu;

impl Mmu {
    pub fn mbc1_read_rom(&self, loc: u16) -> u8 {
        match loc {
            ..0x4000 => self.rom[loc as usize],
            loc => self.rom[loc as usize + 0x4000 * (self.rom_bank as usize - 1)],
        }
    }

    pub fn mbc1_read_ram(&self, loc: u16) -> u8 {
        if self.ram_enable {
            return match self.ram_size {
                0x00 => 0xFF,
                0x02 => self.extern_ram[loc as usize - 0xA000],
                0x03 => self.extern_ram[loc as usize - 0xA000 + 0x2000 * self.ram_bank as usize],
                _ => 0xFF,
            };
        }
        0xFF
    }

    pub fn mbc1_write_rom(&mut self, loc: u16, mut val: u8) {
        match loc {
            ..0x2000 => self.ram_enable = (val & 0xF) == 0xA,
            0x2000..0x4000 => {
                self.rom_bank = (val & 0b11111).max(1);
                self.rom_bank &= (1 << (self.rom_size + 1)) - 1;
            }
            0x4000..0x6000 => {
                if self.ram_size == 0x03 {
                    val &= 0b11;
                    self.ram_bank = val;
                }
            }
            0x6000..0x8000 => {
                self.mbc1_1mb_mode = (val & 1) == 1;
            }
            _ => (),
        }
    }

    pub fn mbc1_write_ram(&mut self, loc: u16, val: u8) {
        if self.ram_enable {
            match self.ram_size {
                0x00 => (),
                0x02 => self.extern_ram[loc as usize - 0xA000] = val,
                0x03 => {
                    self.extern_ram[loc as usize - 0xA000 + 0x2000 * self.ram_bank as usize] = val
                }
                _ => (),
            }
        }
    }
}
