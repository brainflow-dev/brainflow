#[cfg(feature = "generate_binding")]
use std::{env, path::PathBuf};

use std::{fmt::Display, path::Path};

#[cfg(feature = "generate_binding")]
fn generate_binding() {
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    dbg!(&out_path);
    let header_path = out_path.join("inc");
    dbg!(&header_path);
    let header_path = header_path.display();

    dbg!(&header_path);
    dbg!(&out_path);
    let bindings = bindgen::Builder::default()
        // The input header we would like to generate
        // bindings for.
        .header(format!("{}/board_controller.h", &header_path))
        .header(format!("{}/board_info_getter.h", &header_path))
        .header(format!("{}/data_handler.h", &header_path))
        .header(format!("{}/ml_module.h", &header_path))
        .clang_arg("-std=c++11")
        .clang_arg("-x")
        .clang_arg("c++")
        .enable_cxx_namespaces()
        // .opaque_type("std::.*")
        // Tell cargo to invalidate the built crate whenever any of the
        // included header files changed.
        .parse_callbacks(Box::new(bindgen::CargoCallbacks))
        // Finish the builder and generate the bindings.
        .generate()
        // Unwrap the Result and panic on failure.
        .expect("Unable to generate bindings");

    let binding_target_path = PathBuf::new().join("src").join("lib.rs");

    bindings
        .write_to_file(binding_target_path)
        .expect("Could not write binding to `src/lib.rs`");
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
    let brainflow_build_dir = cmake::build(brainflow_path);
    link(brainflow_build_dir.display())
}

fn link(brainflow_build_dir: impl Display) {
    println!("cargo:rustc-link-lib={}=Brainflow", "static");
    println!("cargo:rustc-link-lib={}=DSPFilters", "static");
    println!("cargo:rustc-link-lib={}=BoardController", "dylib");
    println!("cargo:rustc-link-lib={}=DataHandler", "dylib");
    println!("cargo:rustc-link-lib={}=BrainBitLib", "dylib");
    println!("cargo:rustc-link-lib={}=MLModule", "dylib");
    println!("cargo:rustc-link-lib={}=MuseLib", "dylib");
    println!("cargo:rustc-link-lib={}=stdc++", "dylib");
    println!("cargo:rustc-link-search=native={}/lib", brainflow_build_dir);
}

fn main() {
    build();

    #[cfg(feature = "generate_binding")]
    generate_binding();

    println!("cargo:rerun-if-changed=src/lib.rs");
    println!("cargo:rerun-if-changed=build.rs");
}
