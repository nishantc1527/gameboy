use super::Mmu;
use crate::constants::cpu::CPU_FREQ;

impl Mmu {
    pub fn has_rtc(&self) -> bool {
        matches!(self.cart_type, 0x0F | 0x10)
    }

    fn tick_one_second(&mut self) {
        let new_s = self.rtc_s.wrapping_add(1);
        if new_s == 60 {
            self.rtc_s = 0;
            let new_m = self.rtc_m.wrapping_add(1);
            if new_m == 60 {
                self.rtc_m = 0;
                let new_h = self.rtc_h.wrapping_add(1);
                if new_h == 24 {
                    self.rtc_h = 0;
                    let day = (self.rtc_dh_bit as u16) * 256 + self.rtc_dl as u16;
                    let new_day = day + 1;
                    if new_day >= 512 {
                        self.rtc_carry = true;
                    }
                    self.rtc_dl = (new_day & 0xFF) as u8;
                    self.rtc_dh_bit = ((new_day >> 8) & 0x01) as u8;
                } else {
                    self.rtc_h = new_h & 0x1F;
                }
            } else {
                self.rtc_m = new_m & 0x3F;
            }
        } else {
            self.rtc_s = new_s & 0x3F;
        }
    }

    pub fn advance_rtc(&mut self, cycles: u64) {
        if self.rtc_halted || !self.has_rtc() {
            return;
        }
        self.rtc_frac_cycles += cycles;
        let secs = self.rtc_frac_cycles / CPU_FREQ;
        if secs > 0 {
            self.rtc_frac_cycles %= CPU_FREQ;
            for _ in 0..secs {
                self.tick_one_second();
            }
        }
    }

    pub fn advance_rtc_secs(&mut self, n: u64) {
        if n == 0 {
            return;
        }
        let total_s = self.rtc_s as u64 + n;
        let carry_m = total_s / 60;
        self.rtc_s = (total_s % 60) as u8;
        if carry_m == 0 {
            return;
        }
        let total_m = self.rtc_m as u64 + carry_m;
        let carry_h = total_m / 60;
        self.rtc_m = (total_m % 60) as u8;
        if carry_h == 0 {
            return;
        }
        let total_h = self.rtc_h as u64 + carry_h;
        let carry_d = total_h / 24;
        self.rtc_h = (total_h % 24) as u8;
        if carry_d == 0 {
            return;
        }
        let day = (self.rtc_dh_bit as u64) * 256 + self.rtc_dl as u64;
        let new_day = day + carry_d;
        if new_day >= 512 {
            self.rtc_carry = true;
        }
        self.rtc_dl = (new_day & 0xFF) as u8;
        self.rtc_dh_bit = ((new_day >> 8) & 0x01) as u8;
    }

    pub fn rtc_latch(&mut self) {
        self.rtc_latched[0] = self.rtc_s;
        self.rtc_latched[1] = self.rtc_m;
        self.rtc_latched[2] = self.rtc_h;
        self.rtc_latched[3] = self.rtc_dl;
        self.rtc_latched[4] = self.rtc_dh_bit
            | if self.rtc_halted { 0x40 } else { 0x00 }
            | if self.rtc_carry { 0x80 } else { 0x00 };
    }

    pub fn rtc_write_reg(&mut self, bank: u8, val: u8) {
        match bank {
            0x08 => {
                self.rtc_s = val & 0x3F;
                self.rtc_latched[0] = self.rtc_s;
                self.rtc_frac_cycles = 0;
            }
            0x09 => {
                self.rtc_m = val & 0x3F;
                self.rtc_latched[1] = self.rtc_m;
            }
            0x0A => {
                self.rtc_h = val & 0x1F;
                self.rtc_latched[2] = self.rtc_h;
            }
            0x0B => {
                self.rtc_dl = val;
                self.rtc_latched[3] = val;
            }
            0x0C => {
                let halting = val & 0x40 != 0;
                self.rtc_halted = halting;
                self.rtc_carry = val & 0x80 != 0;
                self.rtc_dh_bit = val & 0x01;
                self.rtc_latched[4] = self.rtc_dh_bit
                    | if halting { 0x40 } else { 0x00 }
                    | if self.rtc_carry { 0x80 } else { 0x00 };
            }
            _ => {}
        }
    }

    pub fn mbc3_read_rom(&self, loc: u16) -> u8 {
        match loc {
            ..0x4000 => self.rom[loc as usize],
            0x4000..0x8000 => self.rom[loc as usize + 0x4000 * (self.rom_bank as usize - 1)],
            _ => 0xFF,
        }
    }

    pub fn mbc3_read_ram(&self, loc: u16) -> u8 {
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
            } else if self.has_rtc() && (0x08..=0x0C).contains(&self.ram_bank) {
                return self.rtc_latched[(self.ram_bank - 0x08) as usize];
            }
        }
        0xFF
    }

    pub fn mbc3_write_rom(&mut self, loc: u16, val: u8) {
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
                if !self.has_rtc() {
                    return;
                }
                if val == 0x00 {
                    self.rtc_latch_state = 1;
                } else if val == 0x01 && self.rtc_latch_state == 1 {
                    self.rtc_latch();
                    self.rtc_latch_state = 0;
                }
            }
            _ => (),
        }
    }

    pub fn mbc3_write_ram(&mut self, loc: u16, val: u8) {
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
            } else if self.has_rtc() && (0x08..=0x0C).contains(&self.ram_bank) {
                self.rtc_write_reg(self.ram_bank, val);
            }
        }
    }
}
