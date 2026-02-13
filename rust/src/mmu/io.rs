use std::{
    fs::File,
    io::{Read, Write},
    path::Path,
};

use crate::log_info;

use super::MMU;

impl MMU {
    pub fn save(&self) -> Result<(), std::io::Error> {
        match self.cart_type {
            0x03 | 0x13 => {
                let file_name = format!("{}.sav", self.rom_title);
                let mut file = File::create(Path::new(file_name.as_str()))?;
                file.write_all(&self.extern_ram)?;
                // log_info!("SAVED GAME TO: {}\n", file_name);
            }
            _ => (),
        }
        Ok(())
    }

    pub fn load(&mut self) -> Result<(), std::io::Error> {
        match self.cart_type {
            0x03 | 0x13 => {
                let file_name = format!("{}.sav", self.rom_title);
                let mut file = File::open(Path::new(file_name.as_str()))?;
                file.read(&mut self.extern_ram)?;
                // log_info!("LOADED GAME FROM: {}\n", file_name);
            }
            _ => (),
        }
        Ok(())
    }
}
