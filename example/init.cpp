#include <taskio/log/log.hpp>
#include <taskio/task.hpp>

using taskio::log::log;

taskio::task<> task1() {
    log("task1 satart\n");
    log("task1 end------------------------------\n");
    co_return ;
}

taskio::task<> task2() {
    log("task2 satart\n");

    co_await task1();

    log("task2 resume......\n");
    log("task2 end------------------------------\n");
}

taskio::task<> task3() {
    log("task3 satart\n");

    co_await task2();

    log("task3 resume......\n");
    log("task3 end------------------------------\n");
}

int main() {
    log("main start------------------------------\n");
    auto t = task3();
    t.get_handle().resume();
    return 0;
}