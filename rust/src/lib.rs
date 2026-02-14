mod logging;
mod mmu;

use mmu::MMU;
use std::ffi::{CStr, c_char};

#[unsafe(no_mangle)]
extern "C" fn mmu_init(
    rom_file_name: *const c_char,
    boot_rom_file_name: *const c_char,
    test_category: u8,
) -> *mut MMU {
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
    Box::into_raw(Box::new(
        MMU::new(rom_str, boot_rom_str, test_category)
            .expect("Something went wrong opening the boot rom or rom file"),
    ))
}

#[unsafe(no_mangle)]
extern "C" fn mmu_r_mem(mmu: *const MMU, loc: u16) -> u8 {
    unsafe { (*mmu).r_mem(loc) }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_r_mem_raw(mmu: *const MMU, loc: u16) -> u8 {
    unsafe { (*mmu).r_mem_raw(loc) }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_r_ram_raw(mmu: *const MMU, loc: u16) -> u8 {
    unsafe { (*mmu).r_ram_raw(loc) }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_w_mem(mmu: *mut MMU, loc: u16, val: u8) {
    unsafe { (*mmu).w_mem(loc, val) }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_w_mem_raw(mmu: *mut MMU, loc: u16, val: u8) {
    unsafe {
        (*mmu).w_mem_raw(loc, val);
    }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_w_ram_raw(mmu: *mut MMU, loc: u16, val: u8) {
    unsafe {
        (*mmu).w_ram_raw(loc, val);
    }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_get_rom_title(mmu: *const MMU) -> *const c_char {
    let mmu = unsafe { &*mmu };
    mmu.get_rom_title().as_ptr() as *const c_char
}

#[unsafe(no_mangle)]
extern "C" fn mmu_save(mmu: *const MMU) {
    let save = unsafe { (*mmu).save() };
    match save {
        Ok(_) => (),
        Err(_) => log_err!("COULD NOT SAVE GAME"),
    }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_load(mmu: *mut MMU) {
    let load = unsafe { (*mmu).load() };
    match load {
        Ok(_) => (),
        Err(_) => log_err!("COULD NOT LOAD SAVE FILE"),
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn mmu_free(mmu: *mut MMU) {
    if !mmu.is_null() {
        unsafe {
            _ = Box::from_raw(mmu);
        }
    }
}
