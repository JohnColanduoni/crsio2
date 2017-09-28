use std::path::Path;
use std::fmt;
use std::io;
use std::sync::{Once, ONCE_INIT};
use std::ptr;
use std::ffi::{OsStr, CStr};
#[cfg(target_os = "windows")]
use std::os::windows::ffi::OsStrExt;

#[allow(non_upper_case_globals)]
#[allow(non_camel_case_types)]
#[allow(non_snake_case)]
#[allow(dead_code)]
#[allow(improper_ctypes)]
mod sys {
	include!(concat!(env!("OUT_DIR"), "/sys.rs"));
}

macro_rules! try_sb {
    ($x:expr) => {
        let error: sys::sandbox_error_t = $x;
        if !error.is_null() {
            return Err(Error(error));
        }
    }
}

#[cfg(target_os = "windows")]
#[path = "windows.rs"]
mod platform;

pub use platform::*;

pub struct BrokerServices(sys::sandbox_broker_services_t);
pub struct TargetServices(sys::sandbox_target_services_t);

pub enum Services {
    Broker(BrokerServices),
    Target(TargetServices),
}

pub struct Policy(sys::sandbox_policy_t);

impl Drop for Policy {
    fn drop(&mut self) {
        unsafe { sys::sandbox_policy_release(self.0) }
    }
}

pub struct TargetProcess(sys::sandbox_target_process_t);

impl Drop for TargetProcess {
    fn drop(&mut self) {
        unsafe { sys::sandbox_target_process_release(self.0) }
    }
}

pub struct Error(sys::sandbox_error_t);

impl Drop for Error {
    fn drop(&mut self) {
        unsafe { sys::sandbox_error_release(self.0) }
    }
}

pub type Result<T> = std::result::Result<T, Error>;

impl fmt::Debug for Error {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        let message = unsafe { CStr::from_ptr(sys::sandbox_error_description(self.0)) };

        write!(f, "{}", message.to_string_lossy())
    }
}

static INIT_ONCE: Once = ONCE_INIT;

pub fn init() -> Result<Services> {
    unsafe {
        let mut first_call = false;
        INIT_ONCE.call_once(|| {
            first_call = true;
        });
        if !first_call {
            panic!("sandbox initialization was already performed");
        }

        let broker = sys::sandbox_get_broker_services();
        if !broker.is_null() {
            try_sb!(sys::sandbox_broker_services_init(broker));
            Ok(Services::Broker(BrokerServices(broker)))
        } else {
            let target = sys::sandbox_get_target_services();
            if !target.is_null() {
                try_sb!(sys::sandbox_target_services_init(target));
                Ok(Services::Target(TargetServices(target)))
            } else {
                panic!("failed to initialize sandbox services");
            }
        }
    }
}

impl BrokerServices {
    pub fn create_policy(&mut self) -> Policy {
        unsafe {
            Policy(sys::sandbox_create_policy(self.0))
        }
    }

    pub fn spawn_target<E, C>(
        &mut self,
        executable: &E,
        command_line: &C,
        policy: &Policy,
    ) -> Result<TargetProcess> where
        E: AsRef<Path> + ?Sized,
        C: AsRef<OsStr> + ?Sized,
    {
        unsafe {
            let mut executable: Vec<u16> = executable.as_ref()
                .as_os_str()
                .encode_wide()
                .collect();
            executable.push(0);

            let mut command_line: Vec<u16> = command_line.as_ref()
                .encode_wide()
                .collect();
            command_line.push(0);

            let mut target_process: sys::sandbox_target_process_t = ptr::null_mut();

            try_sb!(sys::sandbox_spawn_target(
                self.0,
                executable.as_ptr(),
                command_line.as_ptr(),
                policy.0,
                &mut target_process,
            ));

            Ok(TargetProcess(target_process))
        }
    }

    pub fn wait_for_all_targets(&mut self) -> Result<()> {
        unsafe {
            try_sb!(sys::sandbox_wait_for_all_targets(self.0));
            Ok(())
        }
    }
}

impl TargetProcess {
    pub fn resume(&mut self) -> Result<()> {
        unsafe {
            try_sb!(sys::sandbox_target_process_resume(self.0));
            Ok(())
        }
    }

    pub fn get_process_id(&self) -> u32 {
        unsafe {
            sys::sandbox_target_process_get_id(self.0)
        }
    }
}

impl TargetServices {
    pub fn lower(&mut self) {
        unsafe {
            sys::sandbox_target_services_lower(self.0);
        }
    }
}

