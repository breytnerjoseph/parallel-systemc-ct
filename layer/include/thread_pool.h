#ifndef THREAD_POOL
#define THREAD_POOL

#include <thread>
#include <future>
#include <mutex>  // std::mutex
#include <condition_variable> // std::condition_variable
#include "threadsafe_queue.h"
#include "function_wrapper.h"
#include "thread_joiner.h"
#include <chrono>
#include <queue>
#include <time.h>

class thread_pool {

    public: 

        thread_pool() : 
            done(false), 
            joiner(threads)
        {
            queue_to_submit = 0;
        }

        ~thread_pool(){
            done=true;
            for(unsigned i = 0; i < thread_count; i++) {
                // std::lock_guard<std::mutex> lck(* thread_mutex[i]);
                task_available_cond_var[i]->notify_one();
            }              
        }


        void launch_threads(unsigned number) {
            thread_count = std::thread::hardware_concurrency(); 
            

            if (number < thread_count) {
                thread_count = number;
            }

            try {
                std::cout << "Launching " << thread_count << " threads" << std::endl;
                for(unsigned i = 0; i < thread_count; i++) {
                    thread_queue.push_back(new std::queue<function_wrapper>);
                    task_available_cond_var.push_back(new std::condition_variable);
                    thread_mutex.push_back(new std::mutex);
                    threads.push_back(std::thread(&thread_pool::worker_thread2, this, i));
                }

                // threads.push_back(std::thread(&thread_pool::dispatch_thread, this));
            }
            catch(...) {
                done = true;
                throw;
            }
        }


        template<typename FunctionType>
        std::future<typename std::result_of<FunctionType()>::type> submit(FunctionType&& f) {
            typedef typename std::result_of<FunctionType()>::type result_type;

            // Notice that result_type() is the signature of the function f:
            // it returns an object of type resut_type and takes no argument
            std::packaged_task<result_type()> task(std::move(f));

            std::future<result_type> res(task.get_future());

            // std::lock_guard<std::mutex>  lk(queue_mutex);
            //work_queue.push(std::move(task));

            unsigned i = queue_to_submit;
            thread_queue[i]->push(std::move(task));
            queue_to_submit = (queue_to_submit + 1) % thread_count;

            // std::lock_guard<std::mutex> lck(* thread_mutex[i]);
           


            return res;
        }


       void dispatch() {
          for(int i = 0; i < thread_count; i++) {
            task_available_cond_var[i]->notify_one();
          }
        }


    private: 
        std::atomic_bool done;
        // bool done;
        threadsafe_queue<function_wrapper> work_queue;
        std::vector<std::queue<function_wrapper> *> thread_queue;
        std::vector<std::thread> threads;
        thread_joiner joiner;
        unsigned thread_count;
        


        // The following two lines are variables that help 
        // replace thread::yield to improve performace: 
        // when a task is submited to the pool, a thread is notify 
        std::mutex queue_mutex;

        std::vector<std::mutex *> thread_mutex;

        std::vector<std::condition_variable *>  task_available_cond_var; 

        unsigned queue_to_submit;
     

        void worker_thread2(int i) {

            function_wrapper task;
            // std::unique_lock<std::mutex> lck(* thread_mutex[i]);

            while (true) {
                while (!thread_queue[i]->empty()) {                    
                    task=std::move(thread_queue[i]->front());
                    thread_queue[i]->pop();
                    task();
                }

                std::unique_lock<std::mutex> lck(* thread_mutex[i]);
                
                // task_available_cond_var[i]->wait(lck,
                // [&]{ 
                //     return !thread_queue[i]->empty() || done;
                //     // return true; //done;
                // });

                task_available_cond_var[i]->wait(lck);

                // std::this_thread::yield();

                //  lck.unlock();
   
                if (done) {
                    return;
                }

            }

        }

        void worker_thread() {
             function_wrapper task;
 
            // while(true) {
                
            //     while (work_queue.try_pop(task)) {
                    
            //         task();

            //         if (!work_queue.empty()) {
            //             task_available_cond_var.notify_one();
            //         }

            //     }

            //         // // Wait for the condition variable of a task 
            //         // // // being available to resume execution
            //         std::unique_lock<std::mutex> lck(queue_mutex);
            //         // // task_available_cond_var.wait_for(lck, std::chrono::milliseconds(1), [&]{ return done || !work_queue.empty();});
                    
            //         // // work_queue.wait_and_pop(task);
                    
                  

            //         // task_available_cond_var.wait(lck);

            //         task_available_cond_var.wait(lck,
            //         [&]{ 
            //             // count2++;
            //             // if (!work_queue.empty() || done) {
            //             //     return true;                            
            //             // }
            //             // count++;
            //             // return false;

            //             return !work_queue.empty() || done;
            //             // return true;
            //         });
            //         lck.unlock();

                    
            //         if (done) {
            //             return;
            //         }

            //         // lck.unlock();
            //         // // if (done) {
            //         // //     return;
            //         // // }
            //         // std::this_thread::yield();
                
            // }
        }

        void run_pending_task() {

        }

};

#endif