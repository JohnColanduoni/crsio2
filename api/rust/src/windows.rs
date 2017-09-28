use super::*;

use std::mem;
use std::os::raw::c_void;
use std::io;

extern crate winapi;
extern crate kernel32;

use self::winapi::*;
use self::kernel32::*;

#[repr(C)]
pub enum TokenLevel {
    Lockdown = 0,
    Restricted,
    Limited,
    Interactive,
    NonAdmin,
    RestrictedSameAccess,
    Unprotected,
}

impl Policy {
    pub fn set_token_level(&mut self, initial: TokenLevel, lockdown: TokenLevel) -> Result<()> {
        unsafe {
            try_sb!(sys::sandbox_policy_set_token_level(self.0, mem::transmute(initial), mem::transmute(lockdown)));

            Ok(())
        }
    }

    pub fn inherit_stdout(&mut self) -> Result<()> {
        unsafe {
            let handle = GetStdHandle(STD_OUTPUT_HANDLE);
            if handle == INVALID_HANDLE_VALUE {
                panic!("GetStdHandle failed: {}", io::Error::last_os_error());
            }
            self.set_stdout_handle(handle)
        }
    }

    pub unsafe fn set_stdout_handle(&mut self, handle: HANDLE) -> Result<()> {
        try_sb!(sys::sandbox_policy_set_stdout_handle(self.0, handle));

        Ok(())
    }
}