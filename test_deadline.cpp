#include "lms/DeadlineCalculator.h"
#include "lms/Timestamp.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>

static bool near(double a, double b) { return std::abs(a-b) < 0.001; }
int main() {
    constexpr double raw = 85.0, maximum = 100.0;
    const std::string d = "2026-11-07 23:59:00";
    auto at = lms::DeadlineCalculator::calculate(d, d, raw, maximum);
    auto twelve = lms::DeadlineCalculator::calculate(d, "2026-11-08 11:59:00", raw, maximum);
    auto day = lms::DeadlineCalculator::calculate(d, "2026-11-08 23:59:00", raw, maximum);
    auto dayHalf = lms::DeadlineCalculator::calculate(d, "2026-11-09 11:59:00", raw, maximum);
    auto two = lms::DeadlineCalculator::calculate(d, "2026-11-09 23:59:00", raw, maximum);
    auto three = lms::DeadlineCalculator::calculate(d, "2026-11-10 23:59:00", raw, maximum);
    auto beyond = lms::DeadlineCalculator::calculate(d, "2026-11-11 00:00:00", raw, maximum);
    assert(at.daysLate == 0 && near(at.finalMark, 85));
    assert(twelve.daysLate == 0 && near(twelve.finalMark, 85));
    assert(day.daysLate == 1 && near(day.finalMark, 75));
    assert(dayHalf.daysLate == 1 && near(dayHalf.finalMark, 75));
    assert(two.daysLate == 2 && near(two.finalMark, 65));
    assert(three.daysLate == 3 && near(three.finalMark, 55));
    assert(beyond.daysLate == 3 && near(beyond.penalty, raw) && near(beyond.finalMark, 0));
    bool invalid = false;
    try { (void)lms::parseLocalTimestamp("2026-15-42 29:90:00"); }
    catch (const std::invalid_argument&) { invalid = true; }
    assert(invalid);
    assert(lms::normalizeDeadline("2026-11-07") == "2026-11-07 23:59:59");
    std::cout << "All exact-time deadline tests passed.\n";
}
