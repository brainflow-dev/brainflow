use std::{
    env,
    path::{Path, PathBuf},
};

#[cfg(feature = "generate_binding")]
use std::{
    fs::File,
    io::prelude::{Read, Write},
};

#[cfg(feature = "generate_binding")]
#[allow(dead_code)]
fn add_link_attribute(path: &Path, library_name: &str) {
    use regex::Regex;
    let re = Regex::new(r"extern").unwrap();
    let mut file = File::open(path).unwrap();
    let mut s = String::new();
    file.read_to_string(&mut s).unwrap();
    let out = re
        .replace_all(&s, format!("#[link(name = \"{}\")]\nextern", library_name))
        .to_string();
    let mut file = File::create(path).unwrap();
    file.write_all(out.as_bytes()).unwrap();
}

#[cfg(feature = "generate_binding")]
fn generate_board_controller_binding() {
    const ALLOW_UNCONVENTIONALS: &'static str = "#![allow(non_camel_case_types)]\n";

    let header_path = PathBuf::from("inc");
    let header_path = header_path.display();

    let bindings = bindgen::Builder::default()
        .header(format!("{}/board_controller.h", &header_path))
        .header(format!("{}/board_info_getter.h", &header_path))
        .raw_line(ALLOW_UNCONVENTIONALS)
        .clang_arg("-std=c++11")
        .clang_arg("-x")
        .clang_arg("c++")
        .generate()
        .expect("Unable to generate bindings");

    let binding_target_path = PathBuf::new()
        .join("src")
        .join("ffi")
        .join("board_controller.rs");

    bindings
        .write_to_file(binding_target_path.clone())
        .expect("Could not write binding to `src/ffi/board_controller.rs`");

    // add_link_attribute(binding_target_path.as_path(), "BoardController");
}

#[cfg(feature = "generate_binding")]
fn generate_data_handler_binding() {
    const ALLOW_UNCONVENTIONALS: &'static str = "#![allow(non_camel_case_types)]\n";

    let header_path = PathBuf::from("inc");
    let header_path = header_path.display();

    let bindings = bindgen::Builder::default()
        .header(format!("{}/data_handler.h", &header_path))
        .raw_line(ALLOW_UNCONVENTIONALS)
        .clang_arg("-std=c++11")
        .clang_arg("-x")
        .clang_arg("c++")
        .generate()
        .expect("Unable to generate bindings");

    let binding_target_path = PathBuf::new()
        .join("src")
        .join("ffi")
        .join("data_handler.rs");

    bindings
        .write_to_file(binding_target_path.clone())
        .expect("Could not write binding to `src/ffi/data_handler.rs`");

    // add_link_attribute(binding_target_path.as_path(), "DataHandler");
}

#[cfg(feature = "generate_binding")]
fn generate_ml_model_binding() {
    const ALLOW_UNCONVENTIONALS: &'static str = "#![allow(non_camel_case_types)]\n";

    let header_path = PathBuf::from("inc");
    let header_path = header_path.display();

    let bindings = bindgen::Builder::default()
        .header(format!("{}/ml_module.h", &header_path))
        .raw_line(ALLOW_UNCONVENTIONALS)
        .clang_arg("-std=c++11")
        .clang_arg("-x")
        .clang_arg("c++")
        .generate()
        .expect("Unable to generate bindings");

    let binding_target_path = PathBuf::new().join("src").join("ffi").join("ml_model.rs");

    bindings
        .write_to_file(binding_target_path.clone())
        .expect("Could not write binding to `src/ffi/ml_model.rs`");

    // add_link_attribute(binding_target_path.as_path(), "MLModule");
}

#[cfg(feature = "generate_binding")]
fn generate_constants_binding() {
    const ALLOW_UNCONVENTIONALS: &'static str = "#![allow(non_camel_case_types)]\n";

    let header_path = PathBuf::from("inc");
    let header_path = header_path.display();

    let bindings = bindgen::Builder::default()
        .header(format!("{}/brainflow_constants.h", &header_path))
        .raw_line(ALLOW_UNCONVENTIONALS)
        .clang_arg("-std=c++11")
        .clang_arg("-x")
        .clang_arg("c++")
        .default_enum_style(bindgen::EnumVariation::Rust {
            non_exhaustive: false,
        })
        .rustified_non_exhaustive_enum("BrainFlowExitCodes")
        .rustified_non_exhaustive_enum("BoardIds")
        .generate()
        .expect("Unable to generate bindings");

    let binding_target_path = PathBuf::new().join("src").join("ffi").join("constants.rs");

    bindings
        .write_to_file(binding_target_path)
        .expect("Could not write binding to `src/ffi/constants.rs`");
}

#[cfg(feature = "generate_binding")]
fn generate_binding() {
    generate_board_controller_binding();
    generate_data_handler_binding();
    generate_ml_model_binding();
    generate_constants_binding();
}

fn main() {
    #[cfg(feature = "generate_binding")]
    generate_binding();

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    let lib_out_path = Path::new(&out_path);

    let lib_path = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());
    let lib_path = Path::new(&lib_path).join("lib");

    let mut options = fs_extra::dir::CopyOptions::new();
    options.overwrite = true;
    options.copy_inside = true;
    fs_extra::dir::copy(lib_path, lib_out_path, &options).unwrap();

    println!("cargo:rustc-link-search=native={}/lib", out_path.display());
    println!("cargo:rustc-link-lib=dylib=BoardController");
    println!("cargo:rustc-link-lib=dylib=DataHandler");
    println!("cargo:rustc-link-lib=dylib=MLModule");
}
