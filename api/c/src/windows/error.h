#include "crsio2.h"
#include <sandbox/win/src/sandbox_types.h>

namespace crsio2 {
  class ChromiumError : public Error {
    public:

      static Result AsResult(sandbox::ResultCode code) {
        if(code == sandbox::SBOX_ALL_OK) {
          return Result::Ok();
        } else if(code == sandbox::SBOX_ERROR_GENERIC) {
          return Result::Err(new ChromiumError(code, GetLastError()));
        } else {
          return Result::Err(new ChromiumError(code, ERROR_SUCCESS));
        }
      }

      const char* description() override;

      ChromiumError(sandbox::ResultCode code, DWORD win32_err) :
        code_(code),
        win32_err_(win32_err),
        win32_err_string_(nullptr)
      {}

    private:

      virtual ~ChromiumError() {
        if(win32_err_string_ != nullptr)
          LocalFree((void*)win32_err_string_);
      }

      sandbox::ResultCode code_;
      DWORD win32_err_;

      const char* win32_err_string_;
  };
}
