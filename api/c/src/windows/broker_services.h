#include "crsio2.h"
#include <sandbox/win/src/sandbox.h>

namespace crsio2 {
  class BrokerServicesImpl : public BrokerServices {
    public:
      BrokerServicesImpl(sandbox::BrokerServices*);

      Result Init() override;
      std::unique_ptr<Policy> CreatePolicy() override;
      Result SpawnTarget(
          sandbox_file_name_t executable_path,
          sandbox_command_line_t command_line,
          Policy* policy,
          std::unique_ptr<TargetProcess>& process_out
      ) override;
      Result WaitForAllTargets() override;

    private:
      sandbox::BrokerServices* inner_;
  };

  class PolicyImpl : public Policy {
    public:
      PolicyImpl(scoped_refptr<sandbox::TargetPolicy> inner) :
        inner_(inner)
      {}

      scoped_refptr<sandbox::TargetPolicy> CloneInner() {
        return inner_;
      }

      Result SetTokenLevel(TokenLevel initial, TokenLevel lockdown) override;

      Result SetJobLevel(JobLevel level) override;

      Result SetIntegrityLevel(IntegrityLevel level) override;

      Result SetLowBox(const wchar_t *sid) override;

      Result SetStdoutHandle(HANDLE handle) override;
      Result SetStderrHandle(HANDLE handle) override;

    private:
      scoped_refptr<sandbox::TargetPolicy> inner_;
  };

  class TargetProcessImpl : public TargetProcess {
    public:
      TargetProcessImpl(PROCESS_INFORMATION handles) :
        handles_(handles)
      {}
      virtual ~TargetProcessImpl();

      Result Resume() override;
    private:
      PROCESS_INFORMATION handles_;
  };
}
