#pragma once

// This component provides platform-dependent miscellanea.

#include <datadog/expected.h>
#include <datadog/string_view.h>

#include <string>

namespace datadog {
namespace tracing {

// A wrapper around an in-memory file descriptor.
//
// This class provides a simple interface to create an in-memory file, write
// data to it, and seal it to prevent further modifications.
// Currently, this implementation is only available on Linux as it relies on the
// `memfd_create` system call.
class InMemoryFile final {
  /// Internal handle on the in-memory file.
  void* handle_ = nullptr;

  /// Constructs an `InMemoryFile` from an existing handle.
  ///
  /// @param handle A valid handle to an in-memory file.
  InMemoryFile(void* handle);
  friend Expected<InMemoryFile> make(StringView);

 public:
  InMemoryFile(const InMemoryFile&) = delete;
  InMemoryFile& operator=(const InMemoryFile&) = delete;
  InMemoryFile(InMemoryFile&&);
  InMemoryFile& operator=(InMemoryFile&&);

  ~InMemoryFile();

  /// Writes content to the in-memory file and then seals it.
  /// Once sealed, further modifications to the file are not possible.
  ///
  /// @param content The data to write into the in-memory file.
  /// @return `true` if the write and seal operations succeed, `false`
  /// otherwise.
  bool write_then_seal(const std::string& content);

  /// Creates an in-memory file with the given name.
  ///
  /// @param name The name of the in-memoru file.
  /// @return An `InMemoryFile` if successful, or an error on failure.
  static Expected<InMemoryFile> make(StringView name);
};

// Hold host information mainly used for telemetry purposes
// and for identifying a tracer.
struct HostInfo final {
  std::string os;
  std::string os_version;
  std::string hostname;
  std::string cpu_architecture;
  std::string kernel_name;
  std::string kernel_version;
  std::string kernel_release;
};

// Returns host information. Lazy.
HostInfo get_host_info();

std::string get_hostname();

int get_process_id();

std::string get_process_name();

int at_fork_in_child(void (*on_fork)());

}  // namespace tracing
}  // namespace datadog
