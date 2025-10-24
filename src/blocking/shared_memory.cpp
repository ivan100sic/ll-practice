#include <practice/blocking/shared_memory.hpp>

#include <fcntl.h>
#include <stdexcept>
#include <sys/mman.h>
#include <unistd.h>

namespace practice::blocking {

SharedMemory::SharedMemory(const std::string &name, size_t length)
    : m_length(length) {
  m_fd = shm_open(name.c_str(), O_RDWR, 0777);
  if (m_fd == -1) {
    throw std::runtime_error{"Failed to open shared memory: " +
                             std::to_string(errno)};
  }

  if (ftruncate(m_fd, length) == -1) {
    close(m_fd);
    throw std::runtime_error{"Failed to set shared memory size: " +
                             std::to_string(errno)};
  }

  m_ptr = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0);
  if (!m_ptr) {
    close(m_fd);
    throw std::runtime_error{"Failed to create map shared memory: " +
                             std::to_string(errno)};
  }
}

SharedMemory::~SharedMemory() {
  if (m_ptr) {
    munmap(m_ptr, m_length);
  }
  if (m_fd != -1) {
    close(m_fd);
  }
}

SharedMemory::SharedMemory(SharedMemory &&that)
    : m_fd(that.m_fd), m_ptr(that.m_ptr), m_length(that.m_length) {
  that.m_fd = -1;
  that.m_ptr = NULL;
}

SharedMemory &SharedMemory::operator=(SharedMemory &&that) {
  m_fd = that.m_fd;
  m_ptr = that.m_ptr;
  m_length = that.m_length;
  that.m_fd = -1;
  that.m_ptr = NULL;
  return *this;
}

bool SharedMemory::unlink(const std::string &name) {
  return shm_unlink(name.c_str()) == 0;
}

} // namespace practice::blocking
