use std::{
    fs::File,
    io::{Read, Write},
    path::Path,
    time::{SystemTime, UNIX_EPOCH},
};

use super::Mmu;

fn unix_secs() -> u64 {
    SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .unwrap_or_default()
        .as_secs()
}

impl Mmu {
    fn save_name(&self) -> String {
        let clean = self
            .rom_title
            .trim_matches(|c: char| c == '\0' || c.is_control());
        format!("{}.sav", clean)
    }

    fn ram_size(&self) -> Option<usize> {
        match self.ram_size {
            0x02 => Some(0x2000),
            0x03 => Some(0x8000),
            _ => None,
        }
    }

    fn save_rtc(&self, file: &mut File) -> Result<(), std::io::Error> {
        file.write_all(&[
            self.rtc_s,
            self.rtc_m,
            self.rtc_h,
            self.rtc_dl,
            self.rtc_dh_bit,
            (self.rtc_halted as u8) | ((self.rtc_carry as u8) << 1),
        ])?;
        file.write_all(&self.rtc_frac_cycles.to_le_bytes())?;
        file.write_all(&unix_secs().to_le_bytes())?;
        Ok(())
    }

    fn load_rtc(&mut self, file: &mut File) {
        let mut buf = [0u8; 22];
        if file.read(&mut buf).unwrap_or(0) >= 22 {
            self.rtc_s = buf[0];
            self.rtc_m = buf[1];
            self.rtc_h = buf[2];
            self.rtc_dl = buf[3];
            self.rtc_dh_bit = buf[4];
            let flags = buf[5];
            self.rtc_halted = flags & 0x01 != 0;
            self.rtc_carry = flags & 0x02 != 0;
            self.rtc_frac_cycles = u64::from_le_bytes(buf[6..14].try_into().unwrap());
            let save_unix = u64::from_le_bytes(buf[14..22].try_into().unwrap());
            if !self.rtc_halted {
                let elapsed = unix_secs().saturating_sub(save_unix);
                self.advance_rtc_secs(elapsed);
            }
        }
        self.rtc_latch();
    }

    pub fn save(&self) -> Result<(), std::io::Error> {
        let file_name = self.save_name();
        if matches!(self.cart_type, 0x06) {
            let mut file = File::create(Path::new(&file_name))?;
            file.write_all(&self.extern_ram[..0x200])?;
        } else if matches!(self.cart_type, 0x0F) {
            let mut file = File::create(Path::new(&file_name))?;
            self.save_rtc(&mut file)?;
        } else if matches!(self.cart_type, 0x03 | 0x10 | 0x13)
            && let Some(len) = self.ram_size()
        {
            let mut file = File::create(Path::new(&file_name))?;
            file.write_all(&self.extern_ram[..len])?;
            if self.has_rtc() {
                self.save_rtc(&mut file)?;
            }
        }
        Ok(())
    }

    pub fn load(&mut self) -> Result<(), std::io::Error> {
        let file_name = self.save_name();
        if matches!(self.cart_type, 0x06) {
            let mut file = File::open(Path::new(&file_name))?;
            let _ = file.read(&mut self.extern_ram[..0x200])?;
        } else if matches!(self.cart_type, 0x0F) {
            let mut file = File::open(Path::new(&file_name))?;
            self.load_rtc(&mut file);
        } else if matches!(self.cart_type, 0x03 | 0x10 | 0x13)
            && let Some(len) = self.ram_size()
        {
            let mut file = File::open(Path::new(&file_name))?;
            let _ = file.read(&mut self.extern_ram[..len])?;
            if self.has_rtc() {
                self.load_rtc(&mut file);
            }
        }
        Ok(())
    }
}
