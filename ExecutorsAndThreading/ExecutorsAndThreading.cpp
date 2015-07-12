// ExecutorsAndThreading.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SignaledObjectNotifier.h"
#include "BlockingWaitForAsync.h"
#include <system_error>
#include <iostream>
#include <cassert>

using namespace std;

void wait_for_object_test() {
    HANDLE handle = ::CreateEvent(nullptr, FALSE, TRUE, nullptr);

    wait_for_object(handle, 0, 0, [](error_code err) {
        std::cout << "Object signaled completion routine, status:" << err.value() << "\n";
        assert(err.value() == 0);
    });

    // wait enough for notification to be signaled on system thread pool
    ::Sleep(100);
    ::CloseHandle(handle);
}


void generate_blocking_completion_test() {
    try {
        int res = async_foo(false);
    } catch(std::exception& e) {
        std::cout << e.what();
    }
}


int _tmain(int argc, _TCHAR* argv []) {
    //wait_for_object_test();
    generate_blocking_completion_test();
}

