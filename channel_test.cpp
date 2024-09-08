#include "channel.hpp"
#include <gtest/gtest.h>
#include <thread>
#include <memory>
#include <future>

// Simple test to check basic functionality of Channel
TEST(ZeroCapChannelTest, SynchOnPushTest) {
  Channel<int, 0> ch;
  ASSERT_EQ(ch.capacity(), 0);
  std::atomic_bool released = false;
  std::thread t{[&]() {
    ch.push(42);
    released = true;
  }};
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  ASSERT_FALSE(released);
  auto result = ch.pop();
  ASSERT_EQ(result, 42);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  ASSERT_TRUE(released);
  t.join();
}

TEST(ZeroCapChannelTest, SynchOnPopTest) {
  uint32_t limit = 1000000;
  auto l = [&]() -> std::shared_ptr<Channel<int,0>>{
  	auto ch = std::make_shared<Channel<int,0>>();
	std::thread t([&](){
		uint32_t first = 0;
		uint32_t second = 1;
		uint32_t fib = 0;
		while(fib < limit){
			fib = first + second;
			first = second;
			second = fib;
		}
		//return fib number before limit
		ch->push(first);	
	});
	t.detach();
	return ch;
  };
  auto ch = l();
  auto result = ch->pop();
  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(*result, 832040);
}

// Test pushing into the channel when it's full
TEST(ChannelTest, SynchOnFullChannelTest) {
  Channel<int, 3> ch;
  ASSERT_EQ(ch.capacity(), 3);
  std::atomic_bool push = true;
  std::atomic_int val = 0;
  std::atomic_bool finished = false;
  std::thread t([&]() {
    while (push) {
      ch.push(int(val));
      ++val;
    }
    finished = true;
  });
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  // check that producer is blocked
  ASSERT_EQ(val, ch.capacity());
  auto res = ch.pop();
  ASSERT_EQ(*res, 0);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  ASSERT_EQ(val, ch.capacity() + 1);
  push = false;
  ASSERT_FALSE(finished);
  res = ch.pop();
  ASSERT_EQ(*res, 1);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  ASSERT_TRUE(finished);
  t.join();
}

TEST(ChannelTest, CancelPush) {
  Channel<int, 0> ch;
  ASSERT_EQ(ch.capacity(), 0);
  std::atomic_bool finished = false;
  std::thread t([&]() {
      ch.push(8);
    finished = true;
  });
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  ASSERT_FALSE(finished);
  ch.cancel();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  ASSERT_TRUE(finished);
  t.join();
}

TEST(ChannelTest, CancelPop) {
  Channel<int, 0> ch;
  ASSERT_EQ(ch.capacity(), 0);
  std::atomic_bool finished = false;
  std::thread t([&]() {
      ch.pop();
      finished = true;
  });
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  ASSERT_FALSE(finished);
  ch.cancel();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  ASSERT_TRUE(finished);
  t.join();
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
