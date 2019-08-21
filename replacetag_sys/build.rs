extern crate bindgen;

use std::env;
use std::path::PathBuf;

fn main() {
    // Tell cargo to tell rustc to link the system bzip2
    // shared library.
    //println!("cargo:rustc-link-lib=bz2");

    // The bindgen::Builder is the main entry point
    // to bindgen, and lets you build up options for
    // the resulting bindings.
    let bindings = bindgen::Builder::default()
        // The input header we would like to generate
        // bindings for.
        
        .header("../src/format-parse.h")
        
        // Finish the builder and generate the bindings.
        .generate()
        // Unwrap the Result and panic on failure.
        .expect("Unable to generate bindings");

    // Write the bindings to the $OUT_DIR/bindings.rs file.
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");

    let mut builder = cc::Build::new();

    builder
        .file("../src/format-parse.c")
        .file("../src/re.c");
        //.flag_if_supported("-fomit-frame-pointer")
        //.flag_if_supported("-fstrict-aliasing");
    // .flag_if_supported("-fprofile-generate")
    let profile = env::var("PROFILE").unwrap();
    if profile == "release" {
        builder.opt_level(2);
    }

    builder.compile("libreplacetag.a");
}