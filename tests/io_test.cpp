// NOLINTBEGIN
//  Note: this is just a test file to demonstrate how to use the library
//        and is not a part of the library itself.

#include <iostream>
#include <string> 

#include "async_lib/task_factory.h"
#include "io/types.h"

auto read_file_body(Async::IOReadRequest req) -> Async::Unit {
    auto body = std::string(req.copy_buffer().get(), req.size());
    std::cout << "Read from file: " << body << '\n';
    return {};
}

auto main() -> int {
    auto task_factory = Async::TaskFactory(/* N_WORKERS = */ 3);
    auto io_source = task_factory.io_source();
    
    auto file     = std::unique_ptr<FILE, decltype(&fclose)>(fopen("tests/io.txt", "r"), &fclose);
    auto io_req = Async::IOReadRequest(Size(1024), Offset(0));
    io_source.read(file.get(), io_req)
             .map<Async::Unit>(read_file_body)
             .block();

    std::cout << "=== Completed ===" << '\n';
}

// NOLINTEND