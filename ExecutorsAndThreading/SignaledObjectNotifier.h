#pragma once

#include <atomic>
#include <memory>
#include <system_error>
#include "async_types.h"

using namespace std;

// default implementation of wait_op
// sends the notification by directly calling the callback
template <class Handler>
struct	wait_op {
    atomic<HANDLE>	wait_handle_;
    Handler	handler;

    explicit wait_op(Handler handler)
        : wait_handle_(0),
        handler(move(handler)) {
    }

    void send_notification(error_code c) {
        handler(c);
    }
};

template <typename T>
struct	wait_op <promise_handler<T>> {
    atomic<HANDLE>	wait_handle_;
    promise_handler<T>	handler;

    explicit wait_op(promise_handler<T> handler)
        : wait_handle_(0),
        handler(move(handler)) {
    }

    void send_notification(error_code c) {
        if( c.value() != 0) {
            handler.p.set_exception(make_exception_ptr(std::exception("")));
        } else {
            handler.p.set_value();
        }
    }
};

template <class Handler>
void CALLBACK wait_callback(void* param, BOOLEAN timed_out) {
    unique_ptr<wait_op<Handler>>	op(
        static_cast<wait_op<Handler>*>(param));

    // wait until we can get access to handle
    while(op->wait_handle_ == 0)
        SwitchToThread();

    const error_code	ec = timed_out
        ? make_error_code(errc::timed_out)
        : error_code();
    op->send_notification(ec);
};


template
<class CompletionToken>
auto wait_for_object(
HANDLE	object, DWORD	timeout,
DWORD	flags, CompletionToken&& token) ->
async_result_t < handler_type_t<CompletionToken, void(error_code)> > {
    async_completion <CompletionToken, void(error_code)> completion(token);
    typedef	handler_type_t<CompletionToken, void(error_code)>	Handler;

    unique_ptr<wait_op<Handler>> op(new	wait_op<Handler>(move(completion.handler)));

    HANDLE	wait_handle;
    if(RegisterWaitForSingleObject(
        &wait_handle, object,
        &wait_callback<Handler>,
        op.get(), timeout,
        flags | WT_EXECUTEONLYONCE)) {

        // we transfer ownership of handle to the wait_op
        op->wait_handle_ = wait_handle;
        op.release();
    } else {
        DWORD	last_error = GetLastError();
        const	error_code	ec(
            last_error, system_category());
        op->send_notification(ec);
    }

    return	completion.result.get();
};