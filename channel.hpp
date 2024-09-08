#include <condition_variable>
#include <mutex>
#include <queue>
#include <type_traits>
#include <optional>
#include <atomic>

template <typename T, size_t CAP> class Channel {
public:
  Channel() = default;
  ~Channel() = default;

  // Delete the copy constructor and copy assignment operator
  Channel(const Channel &) = delete;            // Non-copyable
  Channel &operator=(const Channel &) = delete; // Non-assignable

  template <size_t N = CAP>
  typename std::enable_if<(N > 0)>::type push(T &&obj) {
    std::unique_lock<std::mutex> lk(mu_);
    push_cv_.wait(lk, [this]() { return queue_.size() < CAP || cancel_; });
    if(cancel_){
      return;
    }
    queue_.push(std::move(obj));
    pop_cv_.notify_all();
  }

  template <size_t N = CAP>
  typename std::enable_if<(N == 0)>::type push(T &&obj) {
    std::unique_lock<std::mutex> lk(mu_);
    push_cv_.wait(lk, [this]() { return (queue_.empty() && !writer_waiting_) || cancel_; });
    if(cancel_){
      return;
    }
    writer_waiting_ = true;
    queue_.push(std::move(obj));
    pop_cv_.notify_all();
    push_cv_.wait(lk, [this]() { return (queue_.empty() && writer_waiting_) || cancel_; });
    if(cancel_){
      return;
    }
    writer_waiting_ = false;
    push_cv_.notify_all();
  }

  std::optional<T> pop() {
    std::unique_lock<std::mutex> lk(mu_);
    pop_cv_.wait(lk, [this]() { return !queue_.empty() || cancel_; });
    if(cancel_){
      return std::nullopt;
    }
    auto ret = std::move(queue_.front());
    queue_.pop();
    push_cv_.notify_all();
    return ret;
  }

  size_t capacity() const { return capacity_; }

  void cancel(){
    std::unique_lock<std::mutex> lk(mu_);
    cancel_ = true;
    pop_cv_.notify_all();
    push_cv_.notify_all();
  }

private:
  std::queue<T> queue_;
  std::mutex mu_;
  std::condition_variable pop_cv_;
  std::condition_variable push_cv_;
  bool writer_waiting_ = false;
  size_t capacity_ = CAP;
  std::atomic_bool cancel_ = false;
};
