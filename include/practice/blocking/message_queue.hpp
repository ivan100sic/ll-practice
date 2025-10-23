#pragma once

#include <fcntl.h>
#include <mqueue.h>
#include <string>

namespace practice::blocking {

/**
 * Creates a bidirectional blocking message queue.
 */
class MessageQueue {
public:
  MessageQueue(const std::string &name);
  ~MessageQueue();

  MessageQueue(const MessageQueue &) = delete;
  MessageQueue &operator=(const MessageQueue &) = delete;

  void send(std::string data) const;
  std::string receive() const;

private:
  mqd_t m_fd = -1;
};

} // namespace practice::blocking