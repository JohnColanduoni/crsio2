
#[allow(non_upper_case_globals)]
#[allow(non_camel_case_types)]
#[allow(non_snake_case)]
#[allow(dead_code)]
#[allow(improper_ctypes)]
mod sys {
	include!(concat!(env!("OUT_DIR"), "/sys.rs"));
}

pub struct BrokerServices(sys::sandbox_broker_services_t);
pub struct TargetServices(sys::sandbox_target_services_t);

pub fn init() -> Result<BrokerServices, TargetServices> {
    unsafe {
        let pointer = sys::sandbox_get_broker_services();
        if !pointer.is_null() {
            Ok(BrokerServices(pointer))
        } else {
            let pointer = sys::sandbox_get_target_services();
            if !pointer.is_null() {
                Err(TargetServices(pointer))
            } else {
                panic!("failed to initialize sandbox services");
            }
        }
    }
}

