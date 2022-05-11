use std::{
    env,
    path::{Path, PathBuf},
};

#[cfg(feature = "generate_binding")]
use convert_case::{Case, Casing};
#[cfg(feature = "generate_binding")]
use std::fs::File;

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
}

#[cfg(feature = "generate_binding")]
fn generate_constants_binding() {
    use std::io::Write;

    let header_path = PathBuf::from("inc");
    let header_path = header_path.display();

    let bindings = bindgen::Builder::default()
        .header(format!("{}/brainflow_constants.h", &header_path))
        .clang_arg("-std=c++11")
        .clang_arg("-x")
        .clang_arg("c++")
        .default_enum_style(bindgen::EnumVariation::Rust {
            non_exhaustive: false,
        })
        .generate()
        .expect("Unable to generate bindings");

    let bindings = bindings.to_string();

    let re = regex::Regex::new(r"\b[A-Z0-9]+(_[A-Z0-9]+)*\b").unwrap();
    let bindings = re.replace_all(&bindings, |s: &regex::Captures| {
        s[0].to_string().to_case(Case::Pascal)
    });

    let bindings = bindings.replace("const First", "const FIRST");
    let bindings = bindings.replace("const Last", "const LAST");
    let bindings = bindings.replace("#[derive(", "#[derive(FromPrimitive, ToPrimitive, ");

    let binding_target_path = PathBuf::new().join("src").join("ffi").join("constants.rs");
    let mut file = File::create(&binding_target_path).unwrap();

    file.write_all(bindings.as_bytes()).unwrap();
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
