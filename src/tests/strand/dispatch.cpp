#include <experimental/executor>
#include <experimental/loop_scheduler>
#include <cassert>
#include <string>

int handler_count = 0;

void handler1()
{
  ++handler_count;
}

struct handler2
{
  handler2() {}
  void operator()() { ++handler_count; }
};

struct handler3
{
  handler3() {}
  handler3(const handler3&) = delete;
  handler3(handler3&&) {}
  void operator()() { ++handler_count; }
};

int handler4()
{
  throw std::runtime_error("oops");
}

int main()
{
  handler2 h2;
  const handler2 ch2;
  handler3 h3;

  std::experimental::loop_scheduler scheduler;
  std::experimental::executor_work_guard<std::experimental::loop_scheduler::executor_type> w(scheduler.get_executor());
  std::experimental::strand<std::experimental::loop_scheduler::executor_type> ex(scheduler.get_executor());
  std::thread t([&](){ scheduler.run(); });

  std::experimental::dispatch(ex, handler1);
  std::experimental::dispatch(ex, &handler1);
  std::experimental::dispatch(ex, handler2());
  std::experimental::dispatch(ex, h2);
  std::experimental::dispatch(ex, ch2);
  std::experimental::dispatch(ex, handler3());
  std::experimental::dispatch(ex, std::move(h3));
  std::future<void> fut1 = std::experimental::dispatch(ex, std::experimental::use_future);
  fut1.get();

  w.reset();
  t.join();

  assert(handler_count == 7);
}
