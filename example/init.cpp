#include <taskio/log/log.hpp>
#include <taskio/task.hpp>

#include <taskio/io_context.hpp>

using taskio::log::log;
using taskio::io_context;

taskio::task<int> task1() {
    log("task1 start\n");
    log("task1 end------------\n");
    co_return 3;
}

taskio::task<> task2() {
    log("task2 start\n");
    auto r = co_await task1();
    log("task2 resume\n");
    log("task1() result is {}\n", r);
    log("task2 end------------\n");
}

taskio::task<> task3() {
    log("task3 start\n");
    co_await task2();
    log("task3 resume\n");
    log("task3 end------------\n");
}

int main() {
    io_context ctx;
    ctx.spawn(task3());
    ctx.start();
}