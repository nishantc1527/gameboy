mod mmu;

use mmu::Mmu;
use std::ffi::{CStr, c_char};

#[repr(C)]
pub enum TestCategory {
    TestAge,
    TestBlarggCpu,
    TestBlarggAudio,
    TestBlarggCpuTime,
    TestBlarggMemTime,
    TestBlarggHaltBug,
    TestBlarggInterruptTime,
    TestBlarggMemTime2,
    TestBlarggOamBug,
    TestBlarggCgbSound,
    TestBully,
    TestAcid2,
    TestGambatte,
    TestMicro,
    TestLittle,
    TestMbc3,
    TestMealybug,
    TestMooneye,
    TestSame,
    TestRtc3Basic,
    TestRtc3Range,
    TestRtc3Sub,
    TestScribble,
    TestStrike,
    TestTurtle,
}

#[unsafe(no_mangle)]
extern "C" fn mmu_init(
    rom_file_name: *const c_char,
    boot_rom_file_name: *const c_char,
    test_category: i8,
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
    match Mmu::new(rom_str, boot_rom_str, test_category) {
        Some(mmu) => Box::into_raw(Box::new(mmu)),
        None => std::ptr::null_mut(),
    }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_r_mem(mmu: *const Mmu, loc: u16) -> u8 {
    unsafe { (*mmu).r_mem(loc) }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_r_mem_raw(mmu: *const Mmu, loc: u16) -> u8 {
    unsafe { (*mmu).r_mem_raw(loc) }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_r_ram_raw(mmu: *const Mmu, loc: u16) -> u8 {
    unsafe { (*mmu).r_ram_raw(loc) }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_w_mem(mmu: *mut Mmu, loc: u16, val: u8) {
    unsafe { (*mmu).w_mem(loc, val) }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_w_mem_raw(mmu: *mut Mmu, loc: u16, val: u8) {
    unsafe {
        (*mmu).w_mem_raw(loc, val);
    }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_w_ram_raw(mmu: *mut Mmu, loc: u16, val: u8) {
    unsafe {
        (*mmu).w_ram_raw(loc, val);
    }
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
extern "C" fn mmu_take_div_reset(mmu: *mut Mmu) -> bool {
    unsafe { (*mmu).take_div_reset() }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_get_vram_bank1_byte(mmu: *const Mmu, addr: u16) -> u8 {
    unsafe { (*mmu).get_vram_bank1_byte(addr) }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_get_bg_pal_byte(mmu: *const Mmu, idx: u8) -> u8 {
    unsafe { (*mmu).get_bg_pal_byte(idx) }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_get_obj_pal_byte(mmu: *const Mmu, idx: u8) -> u8 {
    unsafe { (*mmu).get_obj_pal_byte(idx) }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_do_hdma_block(mmu: *mut Mmu) {
    unsafe { (*mmu).do_hdma_block() }
}

#[unsafe(no_mangle)]
extern "C" fn mmu_set_joypad(mmu: *mut Mmu, btns: u8, dirs: u8) {
    unsafe { (*mmu).set_joypad(btns, dirs) }
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
