#include <practice/blocking/message_queue.hpp>
#include <practice/blocking/shared_memory.hpp>

#include <iostream>

namespace message_queue {

const int NumMessages = 1'000'000;

constexpr char OldQueue[] = "/practice";
constexpr char UpQueueName[] = "/practice_up";
constexpr char DownQueueeName[] = "/practice_down";

using practice::blocking::MessageQueue;

void client() {
  MessageQueue queue_up(UpQueueName);
  MessageQueue queue_down(DownQueueeName);
  for (int i = 0; i < NumMessages; i++) {
    queue_up.send("ping");
    if (queue_down.receive() != "pong") {
      std::cerr << "error: " << i << "\n";
      return;
    }
  }
  std::cerr << "ok\n";
}

void server() {
  MessageQueue queue_up(UpQueueName);
  MessageQueue queue_down(DownQueueeName);
  while (true) {
    if (queue_up.receive() == "ping") {
      queue_down.send("pong");
    }
  }
}

void cleanup() {
  MessageQueue::unlink(OldQueue);
  MessageQueue::unlink(UpQueueName);
  MessageQueue::unlink(DownQueueeName);
}

/**
 * About 58 seconds for 2 million messages
 */
void run(const std::string &mode) {
  if (mode == "client") {
    client();
  } else if (mode == "server") {
    server();
  } else {
    // assume cleanup
    cleanup();
  }
}

} // namespace message_queue

namespace shared_memory {

const int NumMessages = 100'000'000;
constexpr size_t MemSize = 4096;
constexpr char SharedRegionName[] = "/practice_shm";

using practice::blocking::SharedMemory;

void client() {
  SharedMemory memory(SharedRegionName, MemSize);
  auto flag = static_cast<volatile unsigned *>(memory.get());
  auto task = flag + 1;

  *flag = 0;
  *task = 1;

  for (int i = 0; i < NumMessages; i++) {
    *flag = 1;
    while (*flag != 0) {
      // wait - now the task is done
    }
  }

  std::cerr << "ok: " << *task << '\n';
}

void server() {
  SharedMemory memory(SharedRegionName, MemSize);
  auto flag = static_cast<volatile unsigned *>(memory.get());
  auto task = flag + 1;

  while (1) {
    if (*flag == 1) {
      *task = *task * 3;
      *flag = 0;
    }
  }
}

void cleanup() { SharedMemory::unlink(SharedRegionName); }

/**
 * About 8.7 seconds for 100 million round trips.
 */
void run(const std::string &mode) {
  if (mode == "client") {
    client();
  } else if (mode == "server") {
    server();
  } else {
    // assume cleanup
    cleanup();
  }
}

} // namespace shared_memory

int main(int argc, char **argv) {
  if (argc > 3 || argc == 1) {
    std::cerr << "Usage: KernelPingPong <mq|shm> [client|server]";
    return 1;
  }

  std::string type = argv[1];

  std::string mode;
  if (argc == 3) {
    mode = argv[2];
  }

  if (type == "mq") {
    message_queue::run(mode);
  } else if (type == "shm") {
    shared_memory::run(mode);
  }
}
