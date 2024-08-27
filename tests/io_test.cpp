// NOLINTBEGIN
//  Note: this is just a test file to demonstrate how to use the library
//        and is not a part of the library itself.

#include <iostream>
#include <string>
#include <chrono>

#include "async_lib/task_factory.h"

using std::chrono_literals::operator""ms;

auto read_file_body(IO::ReadRequest req) -> Async::Unit {
    auto body = std::string(req.copy_buffer().get(), req.size());
    std::cout << "Read from file: " << body << '\n';
    return {};
}

auto delay_by(Async::TaskTimerSource& timer_source, std::chrono::milliseconds amount) -> std::function<Async::Task<Async::Unit>(Async::Unit)> {
    return [&timer_source, amount](auto value) -> Async::Task<Async::Unit> {
        return timer_source
                .after(amount)
                .map<Async::Unit>([value](__attribute__((unused)) auto _) -> Async::Unit { return value; });
    };
}

auto main() -> int {
    auto task_factory = Async::TaskFactory(/* N_WORKERS = */ 3);
    auto io_source = task_factory.io_source();
    auto timer_source = task_factory.timer_source();
    
    auto file     = std::unique_ptr<FILE, decltype(&fclose)>(fopen("tests/io.txt", "r"), &fclose);
    auto io_req = Async::IO::ReadRequest(Async::IO::Size(1024), Async::IO::Offset(0));
    __attribute__((unused)) auto _ = io_source.read(file.get(), io_req)
             .map<Async::Unit>(read_file_body)
             .bind<Async::Unit>(delay_by(timer_source, 300ms))
             .block();

    std::cout << "=== Completed ===" << '\n';
}

// NOLINTEND