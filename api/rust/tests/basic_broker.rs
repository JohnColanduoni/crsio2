extern crate crsio2;

extern crate rand;

use std::io::{self, Read, Write};
use std::path::PathBuf;
use std::fs;
use std::env;

use crsio2::*;
use rand::Rng;

fn main() {
    match init().unwrap() {
        Services::Broker(mut broker) => {
            let mut policy = broker.create_policy();

            #[cfg(target_os = "windows")]
            {
                policy.set_token_level(TokenLevel::RestrictedSameAccess, TokenLevel::Lockdown).unwrap();
                policy.set_job_level(JobLevel::Lockdown).unwrap();
                policy.set_delayed_integrity_level(IntegrityLevel::Untrusted).unwrap();
                policy.set_alternate_desktop(true).unwrap();
            }

            let random_int: i32 = rand::thread_rng().gen();

            let mut file_path = env::temp_dir();
            file_path.push("target_file.txt");

            println!("launching target process");
            let mut target_process = broker.spawn_target(
                &std::env::current_exe().unwrap(),
                &format!("target_process.exe {:?} {}", file_path, random_int),
                &policy
            ).unwrap();

            println!("resuming target process");
            target_process.resume().unwrap();

            broker.wait_for_all_targets().unwrap();
            println!("target process finished");

            let mut file = fs::File::open(&file_path).unwrap();
            let mut file_contents = String::new();
            
            file.read_to_string(&mut file_contents).unwrap();
            
            assert_eq!(format!("{}", random_int), file_contents);
        },
        Services::Target(mut target) => {
            let file_path = PathBuf::from(env::args().nth(1).unwrap());

            let mut file = fs::File::create(&file_path).unwrap();

            target.lower();

            match fs::File::create("another_file.txt") {
                Err(ref err) if err.kind() == io::ErrorKind::PermissionDenied => {},
                _ => panic!("opening file when running under restricted token should have failed"),
            }

            let random_int: i32 = env::args().nth(2).unwrap().parse().unwrap();
            write!(file, "{}", random_int).unwrap();
        },
    }
}
