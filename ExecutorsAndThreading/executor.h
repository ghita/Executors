#pragma once

template <typename Executor>
class executor_work {
public:
    typedef Executor executor_type;

    explicit executor_work(const executor_type& e) : e(e) {}
    executor_work(const	executor_work&) = default;

    executor_work	operator=(const	executor_type&) = delete;

    executor_type	get_executor()	const {
        return e;
    }

private:
    executor_type e;
};

class inline_executor {
public:
    void work_started();
    void work_finished();

    template<typename Fun>
    void dispatch(Fun&& fun) {
        fun();
    }
};

// just return inline_executor for now
template <typename Fun>
inline_executor get_executor(const Fun&) {
    return inline_executor{};
}