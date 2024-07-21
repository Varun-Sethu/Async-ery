#include <iostream>
#include <string> 

#include "task_factory.h"


auto main() -> int {
    auto task_factory = Async::TaskFactory(/* N_WORKERS = */ 3);
    auto io_source = task_factory.io_source();
    
    auto fp     = std::unique_ptr<FILE, decltype(&fclose)>(fopen("tests/io.txt", "r"), &fclose);
    auto io_req = Async::IOReadRequest(1024, 0);

    auto task = io_source.read(fp.get(), io_req)
                         .map<Async::Unit>([](auto req) {
                            auto body = std::string(req.copy_buffer().get(), req.size());
                            std::cout << "Read from file: " << body << std::endl;
                            return Async::Unit();   
                          });

    task.block();
    std::cout << "=== Completed ===" << std::endl;
}