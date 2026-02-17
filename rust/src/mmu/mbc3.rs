use super::Mmu;

impl Mmu {
    pub(super) fn mbc3_read_rom(&self, loc: u16) -> u8 {
        match loc {
            ..0x4000 => self.rom[loc as usize],
            0x4000..0x8000 => self.rom[loc as usize + 0x4000 * (self.rom_bank as usize - 1)],
            _ => 0xFF,
        }
    }

    pub(super) fn mbc3_read_ram(&self, loc: u16) -> u8 {
        if self.ram_enable {
            if self.ram_bank <= 0x07 {
                return match self.ram_size {
                    0x00 => 0xFF,
                    0x02 => self.extern_ram[(loc - 0xA000) as usize],
                    0x03 => {
                        self.extern_ram[(loc - 0xA000 + 0x2000 * self.ram_bank as u16) as usize]
                    }
                    _ => 0xFF,
                };
            } else if self.ram_bank >= 0x08 && self.ram_bank <= 0x0C {
                // TODO RTC
            }
        }
        0xFF
    }

    pub(super) fn mbc3_write_rom(&mut self, loc: u16, val: u8) {
        match loc {
            ..0x2000 => self.ram_enable = (val & 0xF) == 0xA,
            0x2000..0x4000 => {
                let mask = if self.rom_size >= 0x07 { 0xFF } else { 0x7F };
                self.rom_bank = (val & mask).max(1);
            }
            0x4000..0x6000 => {
                self.ram_bank = val;
            }
            0x6000..0x8000 => {
                // TODO RTC
            }
            _ => (),
        }
    }

    pub(super) fn mbc3_write_ram(&mut self, loc: u16, val: u8) {
        if self.ram_enable {
            if self.ram_bank <= 0x07 {
                match self.ram_size {
                    0x00 => (),
                    0x02 => self.extern_ram[(loc - 0xA000) as usize] = val,
                    0x03 => {
                        self.extern_ram[(loc - 0xA000 + 0x2000 * self.ram_bank as u16) as usize] =
                            val
                    }
                    _ => (),
                }
            } else if self.ram_bank >= 0x08 && self.ram_bank <= 0x0C {
                // TODO RTC
            }
        }
    }
}
