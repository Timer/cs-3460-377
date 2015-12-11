#ifndef FILE_HPP
#define FILE_HPP

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <future>
#include <string>
#include <windows.h>

std::future<std::string> read_file_async(const char *path) {
  HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
  if (file == INVALID_HANDLE_VALUE) {
    throw std::system_error(GetLastError(), std::system_category());
  }

  auto size = GetFileSize(file, nullptr);
  if (size < 1) {
    std::promise<std::string> p;
    p.set_value("");
    return p.get_future();
  }

  struct param_t {
    OVERLAPPED ol;
    std::string str;
    std::promise<std::string> p;
  };

  auto param = new param_t;
  memset(&param->ol, 0, sizeof(OVERLAPPED));

  param->str.resize(size);

  auto f = param->p.get_future();

  auto io = CreateThreadpoolIo(file, [](PTP_CALLBACK_INSTANCE, PVOID, PVOID Overlapped, ULONG IoResult, ULONG_PTR, PTP_IO Io) {
    auto param = (param_t *) Overlapped;

    if (IoResult) {
      param->p.set_exception(std::make_exception_ptr(std::system_error(GetLastError(), std::system_category())));
    } else {
      param->p.set_value(std::move(param->str));
    }

    delete param;
    CloseThreadpoolIo(Io);
  }, nullptr, nullptr);

  if (!io) {
    param->p.set_exception(std::make_exception_ptr(std::system_error(GetLastError(), std::system_category())));
    return f;
  }

  StartThreadpoolIo(io);

  if (!ReadFile(file, &param->str.front(), size, nullptr, &param->ol)) {
    auto err = GetLastError();
    if (err != ERROR_IO_PENDING) {
      CancelThreadpoolIo(io);
      CloseThreadpoolIo(io);
      param->p.set_exception(std::make_exception_ptr(std::system_error(GetLastError(), std::system_category())));
    }
  }

  return f;
}

#endif
