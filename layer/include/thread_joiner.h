#ifndef THREAD_JOINER
#define THREAD_JOINER

#include <vector>
#include <thread>

class thread_joiner {

    public:
        explicit thread_joiner(std::vector<std::thread>& threads_):
            threads(threads_)
        {
        }
       
        ~thread_joiner() {
            for (unsigned long i = 0; i < threads.size(); i++) {
                if (threads[i].joinable()) {
                    threads[i].join();
                }
            }
        }

    private: 
        std::vector<std::thread>& threads;

};

#endif