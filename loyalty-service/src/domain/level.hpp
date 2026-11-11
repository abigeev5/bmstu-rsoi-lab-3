#pragma once

#include <string_view>

namespace loyalty {

struct Level {
    std::string_view status;
    int discount{};
};

Level LevelFor(int reservation_count);

}
