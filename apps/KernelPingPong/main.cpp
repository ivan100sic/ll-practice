#include <practice/blocking/message_queue.hpp>

#include <iostream>

const int NumMessages = 1'000'000;
constexpr char OldQueue[] = "/practice";
constexpr char UpQueueName[] = "/practice_up";
constexpr char DownQueueeName[] = "/practice_down";

/**
 * About 58 seconds for 2 million messages
 */
int main(int argc, char **argv) {
  using practice::blocking::MessageQueue;

  if (argc > 2) {
    std::cerr << "Usage: KernelPingPong [client|server]";
    return 1;
  }

  std::string mode;
  if (argc == 2) {
    mode = argv[1];
  }

  if (mode == "client") {
    MessageQueue queue_up(UpQueueName);
    MessageQueue queue_down(DownQueueeName);
    for (int i = 0; i < NumMessages; i++) {
      queue_up.send("ping");
      if (queue_down.receive() != "pong") {
        std::cerr << "error: " << i << "\n";
        return 2;
      }
    }
    std::cerr << "ok\n";
    return 0;
  } else if (mode == "server") {
    MessageQueue queue_up(UpQueueName);
    MessageQueue queue_down(DownQueueeName);
    while (true) {
      if (queue_up.receive() == "ping") {
        queue_down.send("pong");
      }
    }
  } else {
    // assume cleanup
    MessageQueue::unlink(OldQueue);
    MessageQueue::unlink(UpQueueName);
    MessageQueue::unlink(DownQueueeName);
  }
}
