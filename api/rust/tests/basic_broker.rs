extern crate crsio2;

use crsio2::*;

fn main() {
    match init().unwrap() {
        Services::Broker(mut broker) => {
            let mut policy = broker.create_policy();

            #[cfg(target_os = "windows")]
            {
                policy.set_token_level(TokenLevel::Unprotected, TokenLevel::Lockdown).unwrap();
            }

            println!("launching target process");
            let mut target_process = broker.spawn_target(
                &std::env::current_exe().unwrap(),
                "",
                &policy
            ).unwrap();

            println!("resuming target process");
            target_process.resume().unwrap();

            broker.wait_for_all_targets().unwrap();
            println!("fin");
        },
        Services::Target(mut target) => {
            println!("hello from target land!");
            target.lower();
            println!("locked down");
        },
    }
}
