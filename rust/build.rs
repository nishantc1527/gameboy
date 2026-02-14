use std::env;
use std::path::PathBuf;

fn main() {
    let sdl3 = pkg_config::Config::new()
        .atleast_version("3.0")
        .probe("sdl3")
        .expect("Couldn't find SDL3 via pkg-config");
    let bindings = bindgen::Builder::default()
        .header("sdl_wrapper.h")
        .clang_args(
            sdl3.include_paths
                .iter()
                .map(|path| format!("-I{}", path.to_string_lossy())),
        )
        .parse_callbacks(Box::new(bindgen::CargoCallbacks::new()))
        .generate()
        .expect("Couldn't generate bindings");
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("sdl_bindings.rs"))
        .expect("Couldn't write bindings");
}
