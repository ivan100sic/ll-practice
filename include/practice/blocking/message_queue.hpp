#pragma once

#include <mqueue.h>
#include <string>

namespace practice::blocking {

/**
 * Creates a bidirectional blocking message queue.
 */
class MessageQueue {
public:
  MessageQueue(std::string name);
  ~MessageQueue();

  MessageQueue(const MessageQueue &) = delete;
  MessageQueue &operator=(const MessageQueue &) = delete;

  MessageQueue(MessageQueue &&);
  MessageQueue &operator=(MessageQueue &&);

  bool send(std::string_view data) const;
  std::string receive() const;
  bool unlink() const;

private:
  std::string m_name;
  mqd_t m_fd = -1;
};

} // namespace practice::blocking
