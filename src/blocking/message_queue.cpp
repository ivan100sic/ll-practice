#include <practice/blocking/message_queue.hpp>

#include <fcntl.h>
#include <stdexcept>

namespace practice::blocking {

MessageQueue::MessageQueue(const std::string &name) {
  m_fd = mq_open(name.c_str(), O_RDWR | O_CREAT);
  if (m_fd == -1) {
    throw std::runtime_error{"Failed to open message queue"};
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

bool MessageQueue::send(std::string data) const {
  int res = mq_send(m_fd, data.data(), data.size(), 0);
  return res == 0;
}

std::string MessageQueue::receive() const {
  char buf[512];
  int bytes = mq_receive(m_fd, buf, sizeof(buf), NULL);
  if (bytes == -1) {
    throw std::runtime_error{"Failed to receive message"};
  } else {
    return std::string(buf, buf + bytes);
  }
}

} // namespace practice::blocking
