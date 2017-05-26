extern crate bindgen;
extern crate glob;

use std::path::{Path, PathBuf};
use std::fs;
use std::env;
use std::process::Command;

fn main() {
    let out_dir: PathBuf = env::var("OUT_DIR").unwrap().into();
    let target = env::var("TARGET").unwrap();
    let is_debug = env::var("DEBUG").unwrap() == "true";

    // Build crsio2 via cmake
    let src_dir = env::current_dir().unwrap().join("../../");
    let cmake_build_dir = out_dir.join("cmake-build");
    fs::create_dir_all(&cmake_build_dir).unwrap();
    let mut cmake_command = Command::new("cmake");
    cmake_command
        .current_dir(&cmake_build_dir);

    if target.contains("windows") {
        if target.contains("x86_64") {
            cmake_command
                .arg("-DCMAKE_GENERATOR_PLATFORM=x64");
        } else {
            cmake_command
                .arg("-DCMAKE_GENERATOR_PLATFORM=x86");
        }
    }

    cmake_command.arg(&src_dir);
    call_command(&mut cmake_command);

    call_command(Command::new("cmake")
        .arg("--build").arg(&cmake_build_dir)
    );
    if target.contains("windows") {
        if is_debug {
            println!("cargo:rustc-link-search=native={}/chromium/Debug", cmake_build_dir.to_str().unwrap());
            println!("cargo:rustc-link-search=native={}/api/c/Debug", cmake_build_dir.to_str().unwrap());
        } else {
            println!("cargo:rustc-link-search=native={}/chromium/Release", cmake_build_dir.to_str().unwrap());
            println!("cargo:rustc-link-search=native={}/api/c/Release", cmake_build_dir.to_str().unwrap());
        }
    } else {
        println!("cargo:rustc-link-search=native={}", cmake_build_dir.to_str().unwrap());
    }
    println!("cargo:rustc-link-lib=static=crsio2");
    println!("cargo:rustc-link-lib=static=chromium_sandbox");
    if target.contains("windows") {
        println!("cargo:rustc-link-lib=dylib=user32");
        println!("cargo:rustc-link-lib=dylib=dbghelp");
        println!("cargo:rustc-link-lib=dylib=winmm");
        println!("cargo:rustc-link-lib=dylib=mincore");
        println!("cargo:rustc-link-lib=dylib=psapi");
    }

    // Register sources
    for entry in glob::glob(&format!("{}/api/c/**/*", src_dir.to_str().unwrap())).unwrap() {
        let entry = entry.unwrap();
        println!("cargo:rerun-if-changed={}", entry.to_str().unwrap())
    }

    // Generate bindings
    let sys_rs = out_dir.join("sys.rs");

    let include_dir = src_dir.join("api/c/include");
       
    {
        let mut bindings = bindgen::Builder::default()
            .header(include_dir.join("crsio2.h").to_str().unwrap())
            .clang_arg(format!("-I{}", include_dir.to_str().unwrap()));

        let bindings = bindings
            .no_unstable_rust()
            .derive_debug(false)
            .generate()
            .expect("unable to generate bindings");

        bindings
            .write_to_file(&sys_rs)
            .expect("unable to write bindings")
    }
}

fn call_command(command: &mut Command) {
    let result = command.status();
    match result {
        Ok(code) if code.success() => {},
        Ok(code) => {
            panic!("command failed with exit code {}", code);
        },
        Err(err) => {
            panic!("could not execute command: {}", err)
        }
    }
}
