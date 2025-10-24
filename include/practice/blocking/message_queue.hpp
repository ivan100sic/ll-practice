#pragma once

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

  MessageQueue(MessageQueue &&);
  MessageQueue &operator=(MessageQueue &&);

  bool send(std::string_view data) const;
  std::string receive() const;

  static bool unlink(const std::string &name);

private:
  mqd_t m_fd = -1;
};

} // namespace practice::blocking
