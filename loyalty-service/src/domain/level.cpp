#include "domain/level.hpp"

namespace loyalty {

Level LevelFor(int reservation_count) {
    if (reservation_count >= 20) {
        return Level{.status = "GOLD", .discount = 10};
    }
    if (reservation_count >= 10) {
        return Level{.status = "SILVER", .discount = 7};
    }
    return Level{.status = "BRONZE", .discount = 5};
}

}
