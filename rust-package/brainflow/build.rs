#[cfg(feature = "generate_binding")]
use std::path::PathBuf;

#[cfg(feature = "generate_binding")]
fn generate_board_controller_binding() {
    const ALLOW_UNCONVENTIONALS: &'static str = "#![allow(non_camel_case_types)]\n";

    let header_path = PathBuf::from("inc");
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

    let binding_target_path = PathBuf::new()
        .join("src")
        .join("ffi")
        .join("board_controller.rs");

    bindings
        .write_to_file(binding_target_path)
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
        .dynamic_library_name("DataHandler")
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
        .write_to_file(binding_target_path)
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
        .dynamic_library_name("MlModule")
        .clang_arg("-std=c++11")
        .clang_arg("-x")
        .clang_arg("c++")
        .generate()
        .expect("Unable to generate bindings");

    let binding_target_path = PathBuf::new().join("src").join("ffi").join("ml_model.rs");

    bindings
        .write_to_file(binding_target_path)
        .expect("Could not write binding to `src/ffi/ml_model.rs`");
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
}
