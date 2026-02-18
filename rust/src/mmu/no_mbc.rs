use super::Mmu;

impl Mmu {
    pub fn no_mbc_read_rom(&self, loc: u16) -> u8 {
        self.rom[loc as usize]
    }
    pub fn no_mbc_read_ram(&self, _loc: u16) -> u8 {
        0xFF
    }
    pub fn no_mbc_write_rom(&self, _loc: u16, _val: u8) {}
    pub fn no_mbc_write_ram(&self, _loc: u16, _val: u8) {}
}
