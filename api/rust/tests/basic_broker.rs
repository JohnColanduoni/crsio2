extern crate crsio2;

use crsio2::*;

fn main() {
    match init().unwrap() {
        Services::Broker(mut broker) => {
            let policy = broker.create_policy();
            let mut target_process = broker.spawn_target(
                &std::env::current_exe().unwrap(),
                "",
                &policy
            ).unwrap();
            target_process.resume().unwrap();
            broker.wait_for_all_targets().unwrap();
        },
        Services::Target(mut target) => {
            target.lower();
        },
    }
}
