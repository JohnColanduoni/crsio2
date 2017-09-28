#include "crsio2.h"
#include "error.h"
#include <sandbox/win/src/sandbox_factory.h>

#include "broker_services.h"

#include <mutex>

using namespace crsio2;

BrokerServices* BrokerServices::GetInstance() {
  static std::once_flag instance_init;
  static BrokerServicesImpl* instance = nullptr;
  std::call_once(instance_init, []() {
      auto inner_instance = sandbox::SandboxFactory::GetBrokerServices();
      if(inner_instance == nullptr) {
        instance = nullptr;
      } else {
        instance = new BrokerServicesImpl(inner_instance);
      }
  });
  return instance;
}

BrokerServicesImpl::BrokerServicesImpl(sandbox::BrokerServices* inner) : 
  inner_(inner)
{
}

Result BrokerServicesImpl::Init() {
  return ChromiumError::AsResult(inner_->Init());
}

std::unique_ptr<Policy> BrokerServicesImpl::CreatePolicy() {
  return std::make_unique<PolicyImpl>(inner_->CreatePolicy());
}

Result BrokerServicesImpl::SpawnTarget(
    sandbox_file_name_t executable_path,
    sandbox_command_line_t command_line,
    Policy* policy,
    std::unique_ptr<TargetProcess>& process_out
) {
  auto policy_impl = static_cast<PolicyImpl*>(policy);

  PROCESS_INFORMATION process_info;
  sandbox::ResultCode last_warning;
  DWORD last_winerror;
  auto overt_result = inner_->SpawnTarget(
      executable_path,
      command_line,
      policy_impl->CloneInner(),
      &last_warning,
      &last_winerror,
      &process_info
  );

  if(overt_result != sandbox::SBOX_ALL_OK) {
    return ChromiumError::AsResult(overt_result);
  }

  if(last_warning != sandbox::SBOX_ALL_OK) {
    // Drop the process
    TerminateProcess(process_info.hProcess, -1000);
    CloseHandle(process_info.hProcess);
    CloseHandle(process_info.hThread);
    return Result::Err(new ChromiumError(last_warning, last_winerror));
  }

  process_out = std::make_unique<TargetProcessImpl>(process_info);

  return Result::Ok();
}

Result BrokerServicesImpl::WaitForAllTargets() {
  return ChromiumError::AsResult(inner_->WaitForAllTargets());
}

Result PolicyImpl::SetTokenLevel(TokenLevel initial, TokenLevel lockdown) {
  return ChromiumError::AsResult(inner_->SetTokenLevel(static_cast<sandbox::TokenLevel>(initial), static_cast<sandbox::TokenLevel>(lockdown)));
}

Result PolicyImpl::SetJobLevel(JobLevel level) {
  return ChromiumError::AsResult(inner_->SetJobLevel(static_cast<sandbox::JobLevel>(level), 0));
}

Result PolicyImpl::SetIntegrityLevel(IntegrityLevel level) {
  return ChromiumError::AsResult(inner_->SetIntegrityLevel(static_cast<sandbox::IntegrityLevel>(level)));
}

Result PolicyImpl::SetDelayedIntegrityLevel(IntegrityLevel level) {
  return ChromiumError::AsResult(inner_->SetDelayedIntegrityLevel(static_cast<sandbox::IntegrityLevel>(level)));
}


Result PolicyImpl::SetAlternateDesktop(bool alternate_winstation) {
  return ChromiumError::AsResult(inner_->SetAlternateDesktop(alternate_winstation));
}

Result PolicyImpl::SetLowBox(const wchar_t *sid) {
  return ChromiumError::AsResult(inner_->SetLowBox(sid));
}

Result PolicyImpl::SetStdoutHandle(void* handle) {
  return ChromiumError::AsResult(inner_->SetStdoutHandle(handle));
}
Result PolicyImpl::SetStderrHandle(void* handle) {
  return ChromiumError::AsResult(inner_->SetStdoutHandle(handle));
}

Result PolicyImpl::AddHandleToShare(void* handle) {
  inner_->AddHandleToShare(handle);
  return Result::Ok();
}

TargetProcessImpl::~TargetProcessImpl() {
  TerminateProcess(handles_.hProcess, -1000);
  CloseHandle(handles_.hProcess);
  CloseHandle(handles_.hThread);
}

Result TargetProcessImpl::Resume() {
  ResumeThread(handles_.hThread);
  return Result::Ok();
}

sandbox_error_t sandbox_policy_set_token_level(sandbox_policy_t policy, sandbox_token_level_t initial, sandbox_token_level_t lockdown) {
  return policy->SetTokenLevel(initial, lockdown).TakeRaw();
}

sandbox_error_t sandbox_policy_set_job_level(sandbox_policy_t policy, sandbox_job_level_t level) {
  return policy->SetJobLevel(level).TakeRaw();
}

sandbox_error_t sandbox_policy_set_integrity_level(sandbox_policy_t policy, sandbox_integrity_level_t level) {
  return policy->SetIntegrityLevel(level).TakeRaw();
}
sandbox_error_t sandbox_policy_set_delayed_integrity_level(sandbox_policy_t policy, sandbox_integrity_level_t level) {
  return policy->SetDelayedIntegrityLevel(level).TakeRaw();
}

sandbox_error_t sandbox_policy_set_alternate_desktop(sandbox_policy_t policy, bool alternate_winstation) {
  return policy->SetAlternateDesktop(alternate_winstation).TakeRaw();
}

sandbox_error_t sandbox_policy_set_low_box(sandbox_policy_t policy, const wchar_t* sid) {
  return policy->SetLowBox(sid).TakeRaw();
}

sandbox_error_t sandbox_policy_set_stdout_handle(sandbox_policy_t policy, void* handle) {
  return policy->SetStdoutHandle(handle).TakeRaw();
}

sandbox_error_t sandbox_policy_set_stderr_handle(sandbox_policy_t policy, void* handle) {
  return policy->SetStderrHandle(handle).TakeRaw();
}

sandbox_error_t sandbox_policy_add_handle_to_share(sandbox_policy_t policy, void* handle) {
  return policy->AddHandleToShare(handle).TakeRaw();
}

void* sandbox_target_process_get_process_information(sandbox_target_process_t process) {
  return (void*)process->GetProcessInformation();
}
