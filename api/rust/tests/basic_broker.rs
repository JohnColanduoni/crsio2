extern crate crsio2;

extern crate rand;
#[cfg(target_os = "windows")]
extern crate winapi;

use std::mem;
use std::ptr;
use std::io::{self, Read, Write, BufRead};
use std::os::windows::io::FromRawHandle;
use std::path::PathBuf;
use std::fs;
use std::env;

use crsio2::*;
use rand::Rng;


fn main() {
    match init().unwrap() {
        Services::Broker(mut broker) => {
            let mut policy = broker.create_policy();

            let mut file_path_1 = env::temp_dir();
            file_path_1.push("target_file.txt");

            let mut file_path_2 = env::temp_dir();
            file_path_2.push("target_file2.txt");

            #[cfg(target_os = "windows")]
            {
                use winapi::shared::minwindef::{DWORD, TRUE, FALSE};
                use winapi::um::minwinbase::{SECURITY_ATTRIBUTES};
                use winapi::um::namedpipeapi::{CreatePipe};

                policy.set_token_level(TokenLevel::RestrictedSameAccess, TokenLevel::Lockdown).unwrap();
                policy.set_job_level(JobLevel::Lockdown).unwrap();
                policy.set_delayed_integrity_level(IntegrityLevel::Untrusted).unwrap();
                policy.set_alternate_desktop(true).unwrap();
                policy.add_rule(RuleSubSystem::Files, RuleSemantics::FilesAllowAny, &file_path_2).unwrap();

                let target_stdout = unsafe {
                    let mut read_pipe = ptr::null_mut();
                    let mut write_pipe = ptr::null_mut();

                    let mut sec_desc: SECURITY_ATTRIBUTES = mem::zeroed();
                    sec_desc.nLength = mem::size_of_val(&sec_desc) as DWORD;
                    sec_desc.bInheritHandle = TRUE;

                    if CreatePipe(
                        &mut read_pipe,
                        &mut write_pipe,
                        &mut sec_desc,
                        0,
                    ) == FALSE {
                        panic!("CreatePipe failed");
                    }

                    policy.set_stdout_handle(write_pipe).unwrap();
                    policy.set_stderr_handle(write_pipe).unwrap();

                    fs::File::from_raw_handle(mem::transmute(read_pipe))
                };

                ::std::thread::spawn(move || {
                    let mut buffer = String::new();
                    let mut stdout = io::BufReader::new(target_stdout);
                    loop {
                        buffer.clear();
                        match stdout.read_line(&mut buffer) {
                        Ok(_) => {
                            io::stdout().write(buffer.as_bytes()).unwrap();
                        },
                        Err(ref err) if err.kind() == io::ErrorKind::UnexpectedEof => break,
                        Err(err) => panic!("failed to read child stdout: {}", err), 
                        }
                    }
                });
            };

            let random_int: i32 = rand::thread_rng().gen();

            println!("launching target process");
            let mut target_process = broker.spawn_target(
                &std::env::current_exe().unwrap(),
                &format!("target_process.exe {} {:?} {:?}", random_int, file_path_1, file_path_2),
                &policy
            ).unwrap();

            println!("resuming target process");
            target_process.resume().unwrap();

            broker.wait_for_all_targets().unwrap();
            println!("target process finished");

            let mut file1 = fs::File::open(&file_path_1).unwrap();
            let mut file1_contents = String::new();
            file1.read_to_string(&mut file1_contents).unwrap();
            let mut file2 = fs::File::open(&file_path_2).unwrap();
            let mut file2_contents = String::new();
            file2.read_to_string(&mut file2_contents).unwrap();
            
            assert_eq!(format!("{}", random_int), file1_contents);
            assert_eq!(format!("{}", random_int), file2_contents);
        },
        Services::Target(mut target) => {
            println!("in target process");

            let random_int: i32 = env::args().nth(1).unwrap().parse().unwrap();
            let file_path_1 = PathBuf::from(env::args().nth(2).unwrap());
            let file_path_2 = PathBuf::from(env::args().nth(3).unwrap());

            let mut file_1 = fs::File::create(&file_path_1).unwrap();

            target.lower();

            match fs::File::create("another_file.txt") {
                Err(ref err) if err.kind() == io::ErrorKind::PermissionDenied => {},
                _ => panic!("opening file when running under restricted token should have failed"),
            }

            let mut file_2 = fs::File::create(&file_path_2).unwrap();

            write!(file_1, "{}", random_int).unwrap();
            write!(file_2, "{}", random_int).unwrap();
        },
    }
}
