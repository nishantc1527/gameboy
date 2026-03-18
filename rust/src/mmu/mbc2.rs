use super::Mmu;

impl Mmu {
    pub fn mbc2_read_rom(&self, loc: u16) -> u8 {
        match loc {
            ..0x4000 => self.rom[loc as usize],
            loc => {
                let num_banks = 1usize << (self.rom_size as usize + 1);
                let effective_bank = (self.rom_bank as usize) & (num_banks - 1);
                self.rom[effective_bank * 0x4000 + (loc as usize - 0x4000)]
            }
        }
    }

    pub fn mbc2_read_ram(&self, loc: u16) -> u8 {
        if self.ram_enable {
            return 0xF0 | (self.extern_ram[(loc & 0x01FF) as usize] & 0x0F);
        }
        0xFF
    }

    pub fn mbc2_write_rom(&mut self, loc: u16, val: u8) {
        if loc < 0x4000 {
            if (loc >> 8) & 1 == 0 {
                self.ram_enable = (val & 0x0F) == 0x0A;
            } else {
                let bank = val & 0x0F;
                self.rom_bank = if bank == 0 { 1 } else { bank };
            }
        }
    }

    pub fn mbc2_write_ram(&mut self, loc: u16, val: u8) {
        if self.ram_enable {
            self.extern_ram[(loc & 0x01FF) as usize] = val & 0x0F;
        }
    }
}
