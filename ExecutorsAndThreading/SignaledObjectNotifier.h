#pragma once

#include <atomic>
#include <memory>
#include <system_error>
#include "async_types.h"

using namespace std;

template <class Handler>
struct	wait_op {
    atomic<HANDLE>	wait_handle_;
    Handler	handler_;

    explicit wait_op(Handler handler)
        : wait_handle_(0),
        handler_(move(handler)) {
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
    op->handler_(ec);
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
        op->handler_(ec);
    }

    return	completion.result.get();
};