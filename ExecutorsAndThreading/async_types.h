#pragma once
#include <type_traits>
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



