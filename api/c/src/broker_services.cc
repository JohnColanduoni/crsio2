#include "crsio2.h"

using namespace crsio2;

sandbox_broker_services_t sandbox_get_broker_services() {
  return BrokerServices::GetInstance();
}

sandbox_error_t sandbox_broker_services_init(sandbox_broker_services_t broker) {
  return broker->Init().TakeRaw();
}

sandbox_policy_t sandbox_create_policy(sandbox_broker_services_t broker) {
  auto policy = broker->CreatePolicy();
  return policy.release();
}

sandbox_error_t sandbox_spawn_target(
    sandbox_broker_services_t broker,
    sandbox_file_name_t executable_path,
    sandbox_command_line_t command_line,
    sandbox_policy_t policy,
    sandbox_target_process_t* process_out
) {
  std::unique_ptr<TargetProcess> process;
  auto result = broker->SpawnTarget(
      executable_path,
      command_line,
      policy,
      process
  );
  if(result.IsOk()) {
    *process_out = process.release();
    return nullptr;
  } else {
    return result.TakeRaw();
  }
}

sandbox_error_t sandbox_wait_for_all_targets(sandbox_broker_services_t broker) {
  return broker->WaitForAllTargets().TakeRaw();
}

void sandbox_policy_release(sandbox_policy_t policy) {
  delete policy;
}

void sandbox_target_process_release(sandbox_target_process_t process) {
  delete process;
}

sandbox_error_t sandbox_target_process_resume(sandbox_target_process_t process) {
  return process->Resume().TakeRaw();
}
