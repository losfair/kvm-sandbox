#include "error.hpp"
#include <mutex>
#include <stdexcept>
#include <string.h>

std::string strerror_safe(int err) {
    static std::mutex m;
    std::lock_guard<std::mutex> lg(m);
    return std::string(strerror(err));
}

int chk_result(int ret) {
    if(ret < 0) {
        throw std::runtime_error(strerror_safe(-ret));
    }

    return ret;
}
