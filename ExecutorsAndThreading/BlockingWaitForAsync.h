#pragma once
#include "async_types.h"
#include <tuple>
#include <future>

// block handler
struct block_t {
};

template <typename... T>
struct block_handler {
    typedef tuple<T...> value_type;
    promise<value_type> p;

    explicit block_handler(block_t) {}

    template <class... Args>
    void operator()(Args&&... args) {
        p.set_value(value_type(forward<Args>(args)...));
    }
};

template<typename... T>
struct block_handler < error_code, T... > {
    typedef tuple<T...> value_type;
    promise<value_type> p;

    explicit block_handler(block_t) {}

    template <class... Args>
    void operator()(error_code e, Args&&... args) {
        if(e)
            p.set_exception(make_exception_ptr(system_error(e)));
        else
            p.set_value(value_type(forward<Args>(args)...));
    }
};

// specialize handler_type trait for block_handler
template <typename R, typename... Args>
struct handler_type < block_t, R(Args...) > {
    typedef block_handler<decay_t<Args>...> type;
};


template <typename... T>
class async_result < block_handler<T...> > {
    typedef typename block_handler<T...>::value_type value_type;
    future<value_type> f;

    static	void	unpack(tuple<>) {}

    template <typename U>
    static U unpack(tuple<U>	t) {
        return	move(std::get<0>(t));
    }

    template <typename... U>
    static	tuple<U...>	unpack(tuple<U...>	u) {
        return u;
    }

public:
    typedef decltype((unpack) (declval<value_type>())) type;

    explicit async_result(block_handler<T...>& h) :
        f(h.p.get_future()) {
    }

    type get() {
        return (unpack) (f.get());
    }
};

async_result_t<block_handler<error_code, int>> async_foo(bool fail) {
    handler_type_t<block_t, void(std::error_code, int)> handler{block_t{}};
    async_result<decltype(handler)>  result(handler);

    if(fail)
        std::move(handler)(make_error_code(std::errc::invalid_argument), 1);
    else
        std::move(handler)(std::error_code(), 1);

    return result.get();
}

