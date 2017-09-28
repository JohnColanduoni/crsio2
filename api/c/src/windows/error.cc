#include "crsio2.h"

#include "error.h"

using namespace crsio2;

const char* ChromiumError::description() {
  switch(this->code_) {
    case sandbox::SBOX_ALL_OK:
      return "ok";
    case sandbox::SBOX_ERROR_GENERIC:
      FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        this->win32_err_,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&this->win32_err_string_,
        0,
        nullptr
      );
      return this->win32_err_string_;
    case sandbox::SBOX_ERROR_BAD_PARAMS:
      return "bad parameters";
    case sandbox::SBOX_ERROR_UNSUPPORTED:
      return "unsupported";
    case sandbox::SBOX_ERROR_NO_SPACE:
      return "no space";
    case sandbox::SBOX_ERROR_INVALID_IPC:
      return "invalid ipc";
    case sandbox::SBOX_ERROR_FAILED_IPC:
      return "failed ipc";
    case sandbox::SBOX_ERROR_NO_HANDLE:
      return "no handle";
    case sandbox::SBOX_ERROR_UNEXPECTED_CALL:
      return "unexpected call";
    case sandbox::SBOX_ERROR_WAIT_ALREADY_CALLED:
      return "wait already called";
    case sandbox::SBOX_ERROR_CHANNEL_ERROR:
      return "channel error";
    case sandbox::SBOX_ERROR_CANNOT_CREATE_DESKTOP:
      return "cannot create desktop";
    case sandbox::SBOX_ERROR_CANNOT_CREATE_WINSTATION:
      return "cannot create winstation";
    case sandbox::SBOX_ERROR_FAILED_TO_SWITCH_BACK_WINSTATION:
      return "failed to switch back winstation";
    case sandbox::SBOX_ERROR_INVALID_APP_CONTAINER:
      return "invalid AppContainer";
    case sandbox::SBOX_ERROR_INVALID_CAPABILITY:
      return "invalid capability";
    case sandbox::SBOX_ERROR_CANNOT_INIT_APPCONTAINER:
      return "cannot init AppContainer";
    case sandbox::SBOX_ERROR_PROC_THREAD_ATTRIBUTES:
      return "PROC_THREAD_ATTRIBUTES error";
    case sandbox::SBOX_ERROR_CREATE_PROCESS:
      return "process creation error";
    case sandbox::SBOX_ERROR_DELEGATE_PRE_SPAWN:
      return "delegate pre-spawn";
    case sandbox::SBOX_ERROR_ASSIGN_PROCESS_TO_JOB_OBJECT:
      return "could not assign process to job";
    case sandbox::SBOX_ERROR_SET_THREAD_TOKEN:
      return "could not set thread token";
    case sandbox::SBOX_ERROR_GET_THREAD_CONTEXT:
      return "could not get thread context";
    case sandbox::SBOX_ERROR_DUPLICATE_TARGET_INFO:
      return "duplicate target info";
    case sandbox::SBOX_ERROR_SET_LOW_BOX_TOKEN:
      return "failed to set low box token";
    case sandbox::SBOX_ERROR_CREATE_FILE_MAPPING:
      return "failed to create file mapping";
    case sandbox::SBOX_ERROR_DUPLICATE_SHARED_SECTION:
      return "duplicate shared section";
    case sandbox::SBOX_ERROR_MAP_VIEW_OF_SHARED_SECTION:
      return "failed to map view of shared section";
    case sandbox::SBOX_ERROR_APPLY_ASLR_MITIGATIONS:
      return "failed to apply ASLR";
    case sandbox::SBOX_ERROR_SETUP_BASIC_INTERCEPTIONS:
      return "failed to setup basic interceptions";
    case sandbox::SBOX_ERROR_SETUP_INTERCEPTION_SERVICE:
      return "failed to setup interception service";
    case sandbox::SBOX_ERROR_INITIALIZE_INTERCEPTIONS:
      return "failed to initialize interceptions";
    case sandbox::SBOX_ERROR_SETUP_NTDLL_IMPORTS:
      return "failed to setup ntdll imports";
    case sandbox::SBOX_ERROR_SETUP_HANDLE_CLOSER:
      return "failed to setup handle closer";
    case sandbox::SBOX_ERROR_CANNOT_GET_WINSTATION:
      return "cannot get winstation";
    case sandbox::SBOX_ERROR_CANNOT_QUERY_WINSTATION_SECURITY:
      return "cannot query winstation security";
    case sandbox::SBOX_ERROR_CANNOT_GET_DESKTOP:
      return "cannot get desktop";
    case sandbox::SBOX_ERROR_CANNOT_QUERY_DESKTOP_SECURITY:
      return "cannot query desktop security";
    case sandbox::SBOX_ERROR_CANNOT_SETUP_INTERCEPTION_CONFIG_BUFFER:
      return "cannot setup interception config buffer";
    case sandbox::SBOX_ERROR_CANNOT_COPY_DATA_TO_CHILD:
      return "cannot copy data to child";
    case sandbox::SBOX_ERROR_CANNOT_SETUP_INTERCEPTION_THUNK:
      return "cannot setup interception thunk";
    case sandbox::SBOX_ERROR_CANNOT_RESOLVE_INTERCEPTION_THUNK:
      return "cannot resolve interception thunk";
    case sandbox::SBOX_ERROR_CANNOT_WRITE_INTERCEPTION_THUNK:
      return "cannot write interception thunk";
    case sandbox::SBOX_ERROR_CANNOT_FIND_BASE_ADDRESS:
      return "cannot find base address";
    default:
      return "unknown";
  }
}