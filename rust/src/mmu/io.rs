use std::{
    fs::File,
    io::{Read, Write},
    path::Path,
};

use super::Mmu;

impl Mmu {
    fn ram_size(&self) -> Option<usize> {
        match self.ram_size {
            0x02 => Some(0x2000),
            0x03 => Some(0x8000),
            _ => None,
        }
    }

    pub fn save(&self) -> Result<(), std::io::Error> {
        if matches!(self.cart_type, 0x03 | 0x13)
            && let Some(len) = self.ram_size()
        {
            let file_name = format!("{}.sav", self.rom_title);
            let mut file = File::create(Path::new(file_name.as_str()))?;
            file.write_all(&self.extern_ram[..len])?;
            // println!("SAVED GAME TO: {}\n", file_name);
        }
        Ok(())
    }

    pub fn load(&mut self) -> Result<(), std::io::Error> {
        if matches!(self.cart_type, 0x03 | 0x13)
            && let Some(len) = self.ram_size()
        {
            let file_name = format!("{}.sav", self.rom_title);
            let mut file = File::open(Path::new(file_name.as_str()))?;
            let _ = file.read(&mut self.extern_ram[..len])?;
            // println!("LOADED GAME FROM: {}\n", file_name);
        }
        Ok(())
    }
}
