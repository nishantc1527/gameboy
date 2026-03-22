pub mod constants;
mod mmu;

use mmu::Mmu;
use std::ffi::{CStr, c_char};

#[unsafe(no_mangle)]
extern "C" fn mmu_init(
    rom_file_name: *const c_char,
    boot_rom_file_name: *const c_char,
) -> *mut Mmu {
    let rom_str = unsafe {
        CStr::from_ptr(rom_file_name)
            .to_str()
            .expect("Could not read rom file name")
    };
    let boot_rom_str = unsafe {
        CStr::from_ptr(boot_rom_file_name)
            .to_str()
            .expect("Could not read boot rom file name")
    };
    match Mmu::new(rom_str, boot_rom_str) {
        Some(mmu) => Box::into_raw(Box::new(mmu)),
        None => std::ptr::null_mut(),
    }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_read_extern_ram(mmu: *const Mmu, loc: u16) -> u8 {
    unsafe { (*mmu).read_extern_ram(loc) }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_write_extern_ram(mmu: *mut Mmu, loc: u16, val: u8) {
    unsafe { (*mmu).write_extern_ram(loc, val) }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_get_rom_title(mmu: *const Mmu) -> *const c_char {
    let mmu = unsafe { &*mmu };
    mmu.get_rom_title().as_ptr() as *const c_char
}

#[unsafe(no_mangle)]
extern "C" fn mmu_save(mmu: *const Mmu) {
    let save = unsafe { (*mmu).save() };
    match save {
        Ok(_) => (),
        Err(_) => eprintln!("COULD NOT SAVE GAME"),
    }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_load(mmu: *mut Mmu) {
    let load = unsafe { (*mmu).load() };
    match load {
        Ok(_) => (),
        Err(_) => eprintln!("COULD NOT LOAD SAVE FILE"),
    }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_is_cgb(mmu: *const Mmu) -> bool {
    unsafe { (*mmu).is_cgb() }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_is_cgb_compat(mmu: *const Mmu) -> bool {
    unsafe { (*mmu).is_cgb_compat() }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_boot_skipped(mmu: *const Mmu) -> bool {
    unsafe { (*mmu).boot_skipped() }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_read_vram_bank1(mmu: *const Mmu, addr: u16) -> u8 {
    unsafe { (*mmu).read_vram_bank1(addr) }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_advance_rtc(mmu: *mut Mmu, cycles: u64) {
    unsafe { (*mmu).advance_rtc(cycles) }
}

#[unsafe(no_mangle)]
pub extern "C" fn mmu_free(mmu: *mut Mmu) {
    if !mmu.is_null() {
        let raw = mmu;
        _ = unsafe { Box::from_raw(raw) };
    }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_read_vram(mmu: *const Mmu, addr: u16) -> u8 {
    unsafe { (*mmu).read_vram(addr) }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_write_vram(mmu: *mut Mmu, addr: u16, val: u8) {
    unsafe { (*mmu).write_vram(addr, val) }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_set_vram_bank(mmu: *mut Mmu, bank: u8) {
    unsafe { (*mmu).set_vram_bank(bank) }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_get_vram_bank(mmu: *const Mmu) -> u8 {
    unsafe { (*mmu).get_vram_bank() }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_read_wram(mmu: *const Mmu, addr: u16) -> u8 {
    unsafe { (*mmu).read_wram(addr) }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_write_wram(mmu: *mut Mmu, addr: u16, val: u8) {
    unsafe { (*mmu).write_wram(addr, val) }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_set_wram_bank(mmu: *mut Mmu, bank: u8) {
    unsafe { (*mmu).set_wram_bank(bank) }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_get_wram_bank(mmu: *const Mmu) -> u8 {
    unsafe { (*mmu).get_wram_bank() }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_read_oam(mmu: *const Mmu, offset: u16) -> u8 {
    unsafe { (*mmu).read_oam(offset) }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_write_oam(mmu: *mut Mmu, offset: u16, val: u8) {
    unsafe { (*mmu).write_oam(offset, val) }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_read_hram(mmu: *const Mmu, offset: u16) -> u8 {
    unsafe { (*mmu).read_hram(offset) }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_write_hram(mmu: *mut Mmu, offset: u16, val: u8) {
    unsafe { (*mmu).write_hram(offset, val) }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_read_boot(mmu: *const Mmu, addr: u16) -> u8 {
    unsafe { (*mmu).read_boot(addr) }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_boot_active(mmu: *const Mmu) -> bool {
    unsafe { (*mmu).boot_active() }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_disable_boot(mmu: *mut Mmu) {
    unsafe { (*mmu).disable_boot() }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_read_cgb_speed(mmu: *const Mmu) -> u8 {
    unsafe { (*mmu).read_cgb_speed() }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_write_cgb_speed(mmu: *mut Mmu, val: u8) {
    unsafe { (*mmu).write_cgb_speed(val) }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_read_rom(mmu: *const Mmu, addr: u16) -> u8 {
    unsafe { (*mmu).read_rom_region(addr) }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_write_rom(mmu: *mut Mmu, addr: u16, val: u8) {
    unsafe { (*mmu).write_rom_region(addr, val) }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_read_eram(mmu: *const Mmu, addr: u16) -> u8 {
    unsafe { (*mmu).read_eram_region(addr) }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_write_eram(mmu: *mut Mmu, addr: u16, val: u8) {
    unsafe { (*mmu).write_eram_region(addr, val) }
}
