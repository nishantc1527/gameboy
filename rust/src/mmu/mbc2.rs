use super::Mmu;

impl Mmu {
    pub fn mbc2_read_rom(&self, loc: u16) -> u8 {
        match loc {
            ..0x4000 => self.rom[loc as usize],
            loc => {
                // The ROM chip only uses as many address bits as it has banks.
                // Banks written beyond the ROM size wrap at the ROM chip level.
                // "bank 0 → bank 1" substitution only applies to the explicit write of 0.
                let num_banks = 1usize << (self.rom_size as usize + 1);
                let effective_bank = (self.rom_bank as usize) & (num_banks - 1);
                self.rom[effective_bank * 0x4000 + (loc as usize - 0x4000)]
            }
        }
    }

    // MBC2 internal RAM: 512 half-bytes at 0xA000–0xA1FF.
    // Address wraps at 9 bits. Upper nibble is undefined (reads as 0xF0 | nibble).
    pub fn mbc2_read_ram(&self, loc: u16) -> u8 {
        if self.ram_enable {
            return 0xF0 | (self.extern_ram[(loc & 0x01FF) as usize] & 0x0F);
        }
        0xFF
    }

    pub fn mbc2_write_rom(&mut self, loc: u16, val: u8) {
        if loc < 0x4000 {
            if (loc >> 8) & 1 == 0 {
                // Bit 8 of address = 0: RAM enable
                self.ram_enable = (val & 0x0F) == 0x0A;
            } else {
                // Bit 8 of address = 1: ROM bank select (lower 4 bits, 0 → 1)
                // Only the lower 4 bits are used; bank 0 maps to bank 1.
                // No masking by ROM size here — the ROM chip handles address wrapping.
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
