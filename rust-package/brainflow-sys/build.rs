#[cfg(feature = "generate_binding")]
use std::{env, path::PathBuf};

use std::{fmt::Display, path::Path};

#[cfg(feature = "generate_binding")]
fn generate_board_controller_binding() {
    const ALLOW_UNCONVENTIONALS: &'static str = "#![allow(non_camel_case_types)]\n";

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    let header_path = out_path.join("inc");
    let header_path = header_path.display();

    let bindings = bindgen::Builder::default()
        .header(format!("{}/board_controller.h", &header_path))
        .header(format!("{}/board_info_getter.h", &header_path))
        .raw_line(ALLOW_UNCONVENTIONALS)
        .dynamic_library_name("BoardController")
        .clang_arg("-std=c++11")
        .clang_arg("-x")
        .clang_arg("c++")
        .generate()
        .expect("Unable to generate bindings");

    let binding_target_path = PathBuf::new().join("src").join("board_controller.rs");

    bindings
        .write_to_file(binding_target_path)
        .expect("Could not write binding to `src/board_controller.rs`");
}

#[cfg(feature = "generate_binding")]
fn generate_data_handler_binding() {
    const ALLOW_UNCONVENTIONALS: &'static str = "#![allow(non_camel_case_types)]\n";

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    let header_path = out_path.join("inc");
    let header_path = header_path.display();

    let bindings = bindgen::Builder::default()
        .header(format!("{}/data_handler.h", &header_path))
        .raw_line(ALLOW_UNCONVENTIONALS)
        .dynamic_library_name("DataHandler")
        .clang_arg("-std=c++11")
        .clang_arg("-x")
        .clang_arg("c++")
        .generate()
        .expect("Unable to generate bindings");

    let binding_target_path = PathBuf::new().join("src").join("data_handler.rs");

    bindings
        .write_to_file(binding_target_path)
        .expect("Could not write binding to `src/data_handler.rs`");
}

#[cfg(feature = "generate_binding")]
fn generate_ml_model_binding() {
    const ALLOW_UNCONVENTIONALS: &'static str = "#![allow(non_camel_case_types)]\n";

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    let header_path = out_path.join("inc");
    let header_path = header_path.display();

    let bindings = bindgen::Builder::default()
        .header(format!("{}/ml_module.h", &header_path))
        .raw_line(ALLOW_UNCONVENTIONALS)
        .dynamic_library_name("MlModule")
        .clang_arg("-std=c++11")
        .clang_arg("-x")
        .clang_arg("c++")
        .generate()
        .expect("Unable to generate bindings");

    let binding_target_path = PathBuf::new().join("src").join("ml_model.rs");

    bindings
        .write_to_file(binding_target_path)
        .expect("Could not write binding to `src/ml_model.rs`");
}

#[cfg(feature = "generate_binding")]
fn generate_constants_binding() {
    const ALLOW_UNCONVENTIONALS: &'static str = "#![allow(non_camel_case_types)]\n";

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    let header_path = out_path.join("inc");
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

    let binding_target_path = PathBuf::new().join("src").join("constants.rs");

    bindings
        .write_to_file(binding_target_path)
        .expect("Could not write binding to `src/constants.rs`");
}

#[cfg(feature = "generate_binding")]
fn generate_binding() {
    generate_board_controller_binding();
    generate_data_handler_binding();
    generate_ml_model_binding();
    generate_constants_binding();
}

fn build() {
    let brainflow_path = Path::new("brainflow");

    println!(
        "cargo:info=Brainflow source path used: {:?}.",
        brainflow_path
            .canonicalize()
            .expect("Could not canonicalise to absolute path")
    );

    println!("cargo:info=Building Brainflow via CMake.");

    let use_libftdi = if cfg!(feature = "use_libfidi") {
        "ON"
    } else {
        "OFF"
    };

    let use_openmp = if cfg!(feature = "use_openmp") {
        "ON"
    } else {
        "OFF"
    };

    let build_oymotion_sdk = if cfg!(feature = "build_oymotion_sdk") {
        "ON"
    } else {
        "OFF"
    };

    let build_bluetooth = if cfg!(feature = "build_bluetooth") {
        "ON"
    } else {
        "OFF"
    };

    let _brainflow_build_dir = cmake::Config::new(brainflow_path)
        .define("USE_LIBFTDI", use_libftdi)
        .define("USE_OPENMP", use_openmp)
        .define("BUILD_OYMOTION_SDK", build_oymotion_sdk)
        .define("BUILD_BLUETOOTH", build_bluetooth)
        .build();
}

fn main() {
    build();

    #[cfg(feature = "generate_binding")]
    generate_binding();
}
