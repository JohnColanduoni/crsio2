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
    return ChromiumError::AsResult(last_warning);
  }

  process_out = std::make_unique<TargetProcessImpl>(process_info);

  return Result::Ok();
}

Result BrokerServicesImpl::WaitForAllTargets() {
  return ChromiumError::AsResult(inner_->WaitForAllTargets());
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
