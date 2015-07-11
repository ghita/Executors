// ExecutorsAndThreading.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SignaledObjectNotifier.h"
#include <system_error>
#include <iostream>
#include <cassert>

using namespace std;


int _tmain(int argc, _TCHAR* argv []) {
    HANDLE handle = ::CreateEvent(nullptr, FALSE, TRUE, nullptr);

    wait_for_object(handle, 0, 0, [](error_code err) {
        std::cout << "Object signaled completion routine, status:" << err.value() << "\n";
        assert(err.value() == 0);
    });

    // wait enough for notification to be signaled on system thread pool
    ::Sleep(100);
    ::CloseHandle(handle);
}

