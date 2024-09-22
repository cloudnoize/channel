
# C++ Channels

This implementation emulates Go's channel behavior as a synchronization primitive in C++. The channel can be created with a specified capacity, affecting how threads interact with it.

## Channel Behavior

### Zero-Capacity Channel
- Acts like an unbuffered channel.
- A thread pushing to the channel will block until its message is consumed.
- A thread reading from the channel will block until a message is available.

### Channel with Capacity > 0
- Threads can push messages without blocking until capacity is reached.
- Readers block if the channel is empty until a message is pushed.

## Example Usage

```cpp
#include <iostream>
#include <thread>
#include <chrono>
#include "channel.hpp" // Assume the channel code is in this file

// Example function to push data to the channel
void producer(Channel<int, 0>& chan) {
    for (int i = 1; i <= 5; ++i) {
        std::cout << "Producer pushing: " << i << "\n";
        chan.push(std::move(i));
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    chan.cancel(); // Cancel the channel when done
}

// Example function to read data from the channel
void consumer(Channel<int, 0>& chan) {
    while (true) {
        auto data = chan.pop();
        if (data) {
            std::cout << "Consumer received: " << *data << "\n";
        } else {
            break; // Exit when the channel is canceled
        }
    }
}

int main() {
    Channel<int, 0> chan; // Create a channel with capacity 2
    std::thread prod(producer, std::ref(chan));
    std::thread cons(consumer, std::ref(chan));

    prod.join();
    cons.join();

    return 0;
}


```



