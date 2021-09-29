#ifndef DIGITAL_CURLING_TEST_UTIL_HPP
#define DIGITAL_CURLING_TEST_UTIL_HPP

#include "gtest/gtest.h"
#include "digital_curling/digital_curling.hpp"

namespace digital_curling::simulation {

bool operator == (StoneData const& a, StoneData const& b);

bool operator == (StoneCollision const& a, StoneCollision const& b);

} // namespace digital_curling::simulation

#endif // DIGITAL_CURLING_TEST_UTIL_HPP
