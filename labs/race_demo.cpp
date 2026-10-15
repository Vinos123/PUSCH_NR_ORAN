#include <atomic>
#include <iostream>
#include <string>
#include <thread>
int main(int argc,char** argv) {
    if (argc!=2 || (std::string(argv[1])!="safe" && std::string(argv[1])!="unsafe")) return 2;
    constexpr int iterations=100000;
    if (std::string(argv[1])=="unsafe") {
        int count=0;
        auto work=[&] { for(int i=0;i<iterations;++i) ++count; };
        std::thread a(work),b(work); a.join();b.join();
        std::cout << count << " (undefined behavior: data race)\n";
    } else {
        std::atomic<int> count{0};
        auto work=[&] { for(int i=0;i<iterations;++i) count.fetch_add(1,std::memory_order_relaxed); };
        std::thread a(work),b(work); a.join();b.join();
        std::cout << count.load() << " (counter only; not a buffer publication example)\n";
    }
}
