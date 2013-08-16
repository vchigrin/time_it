#include <Windows.h>

#include <string>
#include <string.h>
#include <vector>
#include <iomanip>
#include <iostream>

inline unsigned __int64 TimeToInt64(const FILETIME& file_time) {
  ULARGE_INTEGER i;
  i.LowPart = file_time.dwLowDateTime;
  i.HighPart = file_time.dwHighDateTime;
  return i.QuadPart;
}

void PrintTime(const std::string& name, unsigned __int64 time_value) {
  unsigned __int64 minutes = time_value / (60 * 10000000);
  time_value = time_value % (60 * 10000000);
  double seconds = static_cast<double>(time_value) / 10000000.;
  unsigned __int64 hours = minutes / 60;
  minutes = minutes % 60;
  if (hours > 0) {
    std::cout << name << "\t"
              << hours << "h"
              << minutes << "m"
              << std::setprecision(3) << std::fixed << seconds << "s"
              << std::endl;
  } else {
    std::cout << name << "\t"
              << minutes << "m"
              << std::setprecision(3) << std::fixed << seconds << "s"
              << std::endl;
  }
}

int wmain(int argc, wchar_t* argv[])
{
  if (argc < 2) {
    std::cout << "Usage:" << std::endl << "time_it command [arguments]" << std::endl;
    std::cout << "Measures execution time of command" << std::endl;
    return 1;
  }
  std::wstring command_buffer;
  for (int i = 1; i < argc; ++i) {
    if (i > 1)
      command_buffer += L" ";
    command_buffer += argv[i];
  }
  STARTUPINFOW si;
  PROCESS_INFORMATION pi;
  memset(&si, 0, sizeof(si));
  si.cb = sizeof(si);
  memset(&pi, 0, sizeof(pi));
  const wchar_t* data = command_buffer.c_str();
  std::vector<wchar_t> vt_buffer(data, data + command_buffer.length() + 1);
  BOOL bOk = CreateProcessW(NULL, &vt_buffer[0],
      NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
  if (!bOk || !pi.hProcess) {
    DWORD err = GetLastError();
    std::wcerr << "Failed execute \"" << command_buffer;
    std::wcerr << "\". GetLastError " << err << std::endl;
    return 1;
  }
  WaitForSingleObject(pi.hProcess, INFINITE);
  FILETIME creation_time = {0, 0};
  FILETIME exit_time = {0, 0};
  FILETIME kernel_time = {0, 0};
  FILETIME user_time = {0, 0};
  if (!GetProcessTimes(pi.hProcess, &creation_time, &exit_time, &kernel_time, &user_time)) {
    DWORD err = GetLastError();
    std::wcerr << "GetProcessTimes failed. GetLastError " << err << std::endl;
    return 1;
  }
  unsigned __int64 real_time = TimeToInt64(exit_time) - TimeToInt64(creation_time);
  PrintTime("real", real_time);
  PrintTime("user", TimeToInt64(user_time));
  PrintTime("sys", TimeToInt64(kernel_time));
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  return 0;
}
