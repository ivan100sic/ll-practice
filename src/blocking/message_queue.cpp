#include <practice/blocking/message_queue.hpp>

#include <fcntl.h>
#include <stdexcept>

namespace practice::blocking {

MessageQueue::MessageQueue(std::string name) : m_name(std::move(name)) {
  m_fd = mq_open(m_name.c_str(), O_RDWR | O_CREAT, 0777, NULL);
  if (m_fd == -1) {
    throw std::runtime_error{"Failed to open message queue: " +
                             std::to_string(errno)};
  }
}

MessageQueue::~MessageQueue() {
  if (m_fd != -1) {
    mq_close(m_fd);
  }
}

MessageQueue::MessageQueue(MessageQueue &&that) : m_fd(that.m_fd) {
  that.m_fd = -1;
}

MessageQueue &MessageQueue::operator=(MessageQueue &&that) {
  m_fd = that.m_fd;
  that.m_fd = -1;
  return *this;
}

bool MessageQueue::send(std::string_view data) const {
  int res = mq_send(m_fd, data.data(), data.size(), 0);
  return res == 0;
}

std::string MessageQueue::receive() const {
  char buf[8192];
  int bytes = mq_receive(m_fd, buf, sizeof(buf), NULL);
  if (bytes == -1) {
    throw std::runtime_error{"Failed to receive message: " +
                             std::to_string(errno)};

  } else {
    return std::string(buf, buf + bytes);
  }
}

bool MessageQueue::unlink() const { return mq_unlink(m_name.c_str()) == 0; }

} // namespace practice::blocking
