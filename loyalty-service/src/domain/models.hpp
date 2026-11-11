#pragma once

#include <string>

namespace loyalty {

struct Loyalty {
    int reservation_count{};
    std::string status;
    int discount{};
};

}
