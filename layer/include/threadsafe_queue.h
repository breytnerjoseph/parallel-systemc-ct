#ifndef THREADSAFE_QUEUE
#define THREADSAFE_QUEUE



template<typename T>
class threadsafe_queue {
    
    public:
        threadsafe_queue():
            head(new node), tail(head.get())
        {}

        threadsafe_queue(const threadsafe_queue& other) = delete;
        threadsafe_queue& operator=(const threadsafe_queue&other) = delete;
        
        std::shared_ptr<T> try_pop() {
            std::unique_ptr<node> old_head=try_pop_head();
            return (old_head ? old_head->data : std::shared_ptr<T>());
        }

        bool try_pop(T& value) {
            std::unique_ptr<node> const old_head=try_pop_head(value);
            return old_head != nullptr;
        }

        std::shared_ptr<T> wait_and_pop() {
            std::unique_ptr<node> const old_head=wait_pop_head();
            return old_head->data;
        }

        void wait_and_pop(T& value) {
            std::unique_ptr<node> const old_head=wait_pop_head(value);
        }

        
        void push(T new_value) {
            std::shared_ptr<T> new_data(
                std::make_shared<T>(std::move(new_value)));
           
            std::unique_ptr<node> p(new node);
            {
                std::lock_guard<std::mutex> tail_lock(tail_mutex);
                tail->data=new_data;
                node* const new_tail=p.get();
                tail->next=std::move(p);
                tail=new_tail;
            }
            data_cond.notify_one();
        }
        
        bool empty() {
            std::lock_guard<std::mutex> head_lock(head_mutex);
            return (head.get()==get_tail());
        }

    private:
        struct node {
            std::shared_ptr<T> data;
            std::unique_ptr<node> next;
        };

        std::mutex head_mutex;
        std::unique_ptr<node> head;
        std::mutex tail_mutex;
        node *tail;
        std::condition_variable data_cond;

        node* get_tail(){
            std::lock_guard<std::mutex> tail_lock(tail_mutex);
            return tail;
        }


        std::unique_ptr<node> pop_head() {
            std::unique_ptr<node> old_head=std::move(head);
            head=std::move(old_head->next);
            return old_head;
        }


        std::unique_lock<std::mutex> wait_for_data() {
            std::unique_lock<std::mutex> head_lock(head_mutex);
            data_cond.wait(head_lock,[&]{return head.get()!=get_tail();});
            return std::move(head_lock);
        }

        std::unique_ptr<node> wait_pop_head() {
            std::unique_lock<std::mutex> head_lock(wait_for_data());
            return pop_head();
        }

        std::unique_ptr<node> wait_pop_head(T& value) {
            std::unique_lock<std::mutex> head_lock(wait_for_data());
            value=std::move(*head->data);
            return pop_head();
        }

        std::unique_ptr<node> try_pop_head() {
            std::lock_guard<std::mutex> head_lock(head_mutex);
            if (head.get()==get_tail()) {
                return std::unique_ptr<node>();
            }
            return pop_head();
        }

        std::unique_ptr<node> try_pop_head(T& value) {
            std::lock_guard<std::mutex> head_lock(head_mutex);
            if(head.get()==get_tail()) {
                return std::unique_ptr<node>();
            }
            value=std::move(*head->data);
            return pop_head();
        }

};






// #include <queue>
// #include <memory>
// #include <mutex>
// #include <condition_variable>

// template<typename T>
// class threadsafe_queue {

//     private: 
//         mutable std::mutex mut;
//         std::queue<T> data_queue;
//         std::condition_variable data_cond;

//     public:
//         threadsafe_queue() {}
        
//         threadsafe_queue(threadsafe_queue const& other) {
//             std::lock_guard<std::mutex> lk(other.mut);
//             data_queue = other.data_queue;
//         }

//         void push(T new_value) {
//             std::lock_guard<std::mutex> lk(mut);
//             data_queue.push(new_value);
//             data_cond.notify_one();
//         }

//         void wait_and_pop(T& value) {
//             std::unique_lock<std::mutex> lk(mut);
//             data_cond.wait(lk, [this]{return !data_queue.empty();} );
//             value=data_queue.front();
//             data_queue.pop();
//         }

//         std::shared_ptr<T> wait_and_pop() {
//             std::unique_lock<std::mutex> lk(mut);
//             data_cond.wait(lk, [this]{return !data_queue.empty();} );
//             std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
//             data_queue.pop();
//             return res;
//         }

//         bool try_pop(T& value) {
//             std::lock_guard<std::mutex> lk(mut);
//             if (data_queue.empty()) {
//                 return false;
//             }
//             value = std::move(data_queue.front());
//             data_queue.pop();
//             return true;
//         }

//         std::shared_ptr<T> try_pop() {
//             std::lock_guard<std::mutex> lk(mut);
//             if (data_queue.empty()) {
//                 return std::shared_ptr<T>();
//             }
//             std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
//             data_queue.pop();
//             return res;
//         }

//         bool empty() const {
//             std::lock_guard<std::mutex> lk(mut);
//             return data_queue.empty();
//         }
// };

#endif