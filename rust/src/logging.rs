pub mod sdl {
    #![allow(non_upper_case_globals)]
    #![allow(non_camel_case_types)]
    #![allow(non_snake_case)]
    #![allow(unused)]
    include!(concat!(env!("OUT_DIR"), "/sdl_bindings.rs"));
}

#[macro_export]
macro_rules! log_info {
    ($($arg:tt)*) => {{
        let msg = format!($($arg)*);
        if let Ok(c_msg) = std::ffi::CString::new(msg) {
            unsafe {
                $crate::logging::sdl::SDL_LogInfo(
                    $crate::logging::sdl::SDL_LogCategory_SDL_LOG_CATEGORY_APPLICATION as i32,
                    b"%s\0".as_ptr() as *const std::os::raw::c_char,
                    c_msg.as_ptr()
                );
            }
        }
    }};
}

#[macro_export]
macro_rules! log_err {
    ($($arg:tt)*) => {{
        let msg = format!($($arg)*);
        if let Ok(c_msg) = std::ffi::CString::new(msg) {
            unsafe {
                $crate::logging::sdl::SDL_LogError(
                    $crate::logging::sdl::SDL_LogCategory_SDL_LOG_CATEGORY_ERROR as i32,
                    b"%s\0".as_ptr() as *const std::os::raw::c_char,
                    c_msg.as_ptr()
                );
            }
        }
    }};
}
