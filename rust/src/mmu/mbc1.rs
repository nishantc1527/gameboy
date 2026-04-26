use super::Mmu;

const NINTENDO_LOGO: &[u8] = &[
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
    0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
    0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E,
];

pub fn check_multicart(rom: &[u8], rom_size: u8) -> bool {
    if rom_size != 0x05 {
        return false;
    }
    for bank in 1usize..4 {
        let offset = bank * 0x4000 + 0x0104;
        match rom.get(offset..offset + NINTENDO_LOGO.len()) {
            Some(slice) if slice == NINTENDO_LOGO => {}
            _ => return false,
        }
    }
    true
}

impl Mmu {
    fn rom_bank_mask(&self) -> usize {
        (1usize << (self.rom_size as usize + 1)) - 1
    }

    pub fn mbc1_read_rom(&self, loc: u16) -> u8 {
        let mask = self.rom_bank_mask();
        let secondary = self.ram_bank as usize;
        let primary = self.rom_bank as usize;
        if self.mbc1_multicart {
            let primary4 = primary & 0x0F;
            match loc {
                ..0x4000 => {
                    let bank = if self.mbc1_1mb_mode {
                        (secondary << 4) & mask
                    } else {
                        0
                    };
                    self.rom[loc as usize + 0x4000 * bank]
                }
                loc => {
                    let bank = ((secondary << 4) | primary4) & mask;
                    self.rom[loc as usize - 0x4000 + 0x4000 * bank]
                }
            }
        } else {
            match loc {
                ..0x4000 => {
                    let bank = if self.mbc1_1mb_mode {
                        (secondary << 5) & mask
                    } else {
                        0
                    };
                    self.rom[loc as usize + 0x4000 * bank]
                }
                loc => {
                    let bank = ((secondary << 5) | primary) & mask;
                    self.rom[loc as usize - 0x4000 + 0x4000 * bank]
                }
            }
        }
    }

    pub fn mbc1_read_ram(&self, loc: u16) -> u8 {
        if self.ram_enable {
            let ram_bank = if self.mbc1_1mb_mode {
                self.ram_bank as usize
            } else {
                0
            };
            return match self.ram_size {
                0x00 => 0xFF,
                0x02 => self.extern_ram[loc as usize - 0xA000],
                0x03 => self.extern_ram[(loc as usize - 0xA000) + 0x2000 * ram_bank],
                _ => 0xFF,
            };
        }
        0xFF
    }

    pub fn mbc1_write_rom(&mut self, loc: u16, val: u8) {
        match loc {
            ..0x2000 => self.ram_enable = (val & 0x0F) == 0x0A,
            0x2000..0x4000 => {
                self.rom_bank = (val & 0x1F).max(1);
            }
            0x4000..0x6000 => {
                self.ram_bank = val & 0x03;
            }
            0x6000..0x8000 => {
                self.mbc1_1mb_mode = (val & 1) == 1;
            }
            _ => (),
        }
    }

    pub fn mbc1_write_ram(&mut self, loc: u16, val: u8) {
        if self.ram_enable {
            let ram_bank = if self.mbc1_1mb_mode {
                self.ram_bank as usize
            } else {
                0
            };
            match self.ram_size {
                0x00 => (),
                0x02 => self.extern_ram[loc as usize - 0xA000] = val,
                0x03 => self.extern_ram[(loc as usize - 0xA000) + 0x2000 * ram_bank] = val,
                _ => (),
            }
        }
    }
}
