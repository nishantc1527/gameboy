use super::MMU;

impl MMU {
    pub(super) fn mbc1_read_rom(&self, loc: u16) -> u8 {
        match loc {
            ..0x4000 => self.rom[loc as usize],
            loc => self.rom[(loc + 0x4000 * (self.rom_bank as u16 - 1)) as usize],
        }
    }

    pub(super) fn mbc1_read_ram(&self, loc: u16) -> u8 {
        if self.ram_enable {
            return match self.ram_size {
                0x00 => 0xFF,
                0x02 => self.extern_ram[(loc - 0xA000) as usize],
                0x03 => self.extern_ram[(loc - 0xA000 + 0x2000 * self.ram_bank as u16) as usize],
                _ => 0xFF,
            };
        }
        0xFF
    }

    pub(super) fn mbc1_write_rom(&mut self, loc: u16, mut val: u8) {
        match loc {
            ..0x2000 => self.ram_enable = (val & 0xF) == 0xA,
            0x2000..0x4000 => {
                self.rom_bank = val & 0b11111;
                if self.rom_bank == 0 {
                    self.rom_bank = 1;
                }
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

    pub(super) fn mbc1_write_ram(&mut self, loc: u16, val: u8) {
        if self.ram_enable {
            self.extern_ram[(loc - 0xA000) as usize] = val;
        }
    }
}
