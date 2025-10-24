#pragma once

#include <mqueue.h>
#include <string>

namespace practice::blocking {

/**
 * Creates a shared memory view.
 */
class SharedMemory {
public:
  SharedMemory(const std::string &name, size_t length);
  ~SharedMemory();

  SharedMemory(const SharedMemory &) = delete;
  SharedMemory &operator=(const SharedMemory &) = delete;
  SharedMemory(SharedMemory &&);
  SharedMemory &operator=(SharedMemory &&);

  volatile char *get() const;

  static bool unlink(const std::string &name);

private:
  int m_fd;
  size_t m_length;
  void *m_ptr;
};

} // namespace practice::blocking
