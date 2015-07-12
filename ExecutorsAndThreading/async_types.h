#pragma once
#include <type_traits>
#include <future>
using namespace std;

// default implementation when initiating function is  expected
// to notify completion by a simple callback
template <typename CompletionToken, typename Signature>
struct handler_type {
    typedef CompletionToken type;
};

template<typename	CompletionToken, typename	Signature>
using handler_type_t = typename	handler_type<CompletionToken, Signature>::type;

// default implementation when the result returned from the initiating function
// is void - this coresponds to the simple callback completion
template <typename Handler>
class async_result {
public:
    typedef void type;

    explicit async_result(Handler& h) {/* no op */ }
    type get() { /* no-op*/ }
};

template <typename Handler>
using async_result_t = typename async_result<Handler>::type;

template <class	CompletionToken, class Signature>
struct	async_completion {
    typedef	handler_type_t<CompletionToken, Signature>	handler_type;

    async_completion(remove_reference_t<CompletionToken>&	token)
        : handler(std::forward<CompletionToken>(token)),
        result(handler) {
    }

    handler_type	handler;
    async_result<handler_type>	result;
};


// async result specialization

struct use_future_t {
};

template <typename T>
struct promise_handler {
    typedef T value_type;
    promise<value_type> p;

    promise_handler(use_future_t) {}
    promise_handler(promise_handler&& other) : p(move(other.p)) {}
};

template <typename T>
struct handler_type < use_future_t, T(error_code) > {
    typedef promise_handler<T> type;
};


template <typename T>
class async_result < promise_handler<T> > {
public:
    typedef future<T> type;

    explicit async_result(promise_handler<T>& h) : f(h.p.get_future()) {
    }

    type get() {
        return move(f);
    }

private:
    future<T> f;
};



