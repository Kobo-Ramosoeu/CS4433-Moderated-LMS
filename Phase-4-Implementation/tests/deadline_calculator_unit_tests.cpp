
#include "lms/DeadlineCalculator.h"

#include <catch2/catch_test_macros.hpp>

#include <limits>
#include <stdexcept>

TEST_CASE("Submission at or before deadline receives no penalty",
          "[deadline][normal]")
{
    const auto result = lms::DeadlineCalculator::calculate(
        "2026-11-07 23:59:00",
        "2026-11-07 23:59:00",
        85.0,
        100.0
    );

    REQUIRE(result.daysLate == 0);
    REQUIRE(result.penalty == 0.0);
    REQUIRE(result.finalMark == 85.0);
}

TEST_CASE("Submission less than one full day late receives no penalty",
          "[deadline][boundary]")
{
    const auto result = lms::DeadlineCalculator::calculate(
        "2026-11-07 23:59:00",
        "2026-11-08 11:59:00",
        85.0,
        100.0
    );

    REQUIRE(result.daysLate == 0);
    REQUIRE(result.finalMark == 85.0);
}

TEST_CASE("Late penalty increases by ten percent per completed day",
          "[deadline][penalty]")
{
    SECTION("One day late")
    {
        const auto result = lms::DeadlineCalculator::calculate(
            "2026-11-07 23:59:00",
            "2026-11-08 23:59:00",
            85.0,
            100.0
        );

        REQUIRE(result.daysLate == 1);
        REQUIRE(result.penalty == 10.0);
        REQUIRE(result.finalMark == 75.0);
    }

    SECTION("Two days late")
    {
        const auto result = lms::DeadlineCalculator::calculate(
            "2026-11-07 23:59:00",
            "2026-11-09 23:59:00",
            85.0,
            100.0
        );

        REQUIRE(result.daysLate == 2);
        REQUIRE(result.penalty == 20.0);
        REQUIRE(result.finalMark == 65.0);
    }

    SECTION("Three days late")
    {
        const auto result = lms::DeadlineCalculator::calculate(
            "2026-11-07 23:59:00",
            "2026-11-10 23:59:00",
            85.0,
            100.0
        );

        REQUIRE(result.daysLate == 3);
        REQUIRE(result.penalty == 30.0);
        REQUIRE(result.finalMark == 55.0);
    }
}

TEST_CASE("Submission more than three days late receives zero",
          "[deadline][boundary]")
{
    const auto result = lms::DeadlineCalculator::calculate(
        "2026-11-07 23:59:00",
        "2026-11-11 00:00:00",
        85.0,
        100.0
    );

    REQUIRE(result.daysLate == 3);
    REQUIRE(result.penalty == 85.0);
    REQUIRE(result.finalMark == 0.0);
}

TEST_CASE("Late penalty cannot produce a negative final mark",
          "[deadline][boundary]")
{
    const auto result = lms::DeadlineCalculator::calculate(
        "2026-11-07 23:59:00",
        "2026-11-10 23:59:00",
        15.0,
        100.0
    );

    REQUIRE(result.daysLate == 3);
    REQUIRE(result.finalMark == 0.0);
}

TEST_CASE("Invalid marks are rejected",
          "[deadline][validation]")
{
    const std::string deadline = "2026-11-07 23:59:00";

    SECTION("Negative raw mark")
    {
        REQUIRE_THROWS_AS(
            lms::DeadlineCalculator::calculate(
                deadline, deadline, -1.0, 100.0
            ),
            std::invalid_argument
        );
    }

    SECTION("Raw mark above maximum")
    {
        REQUIRE_THROWS_AS(
            lms::DeadlineCalculator::calculate(
                deadline, deadline, 101.0, 100.0
            ),
            std::invalid_argument
        );
    }

    SECTION("Zero maximum mark")
    {
        REQUIRE_THROWS_AS(
            lms::DeadlineCalculator::calculate(
                deadline, deadline, 0.0, 0.0
            ),
            std::invalid_argument
        );
    }

    SECTION("NaN raw mark")
    {
        REQUIRE_THROWS_AS(
            lms::DeadlineCalculator::calculate(
                deadline, deadline,
                std::numeric_limits<double>::quiet_NaN(), 100.0
            ),
            std::invalid_argument
        );
    }

    SECTION("Infinite maximum mark")
    {
        REQUIRE_THROWS_AS(
            lms::DeadlineCalculator::calculate(
                deadline, deadline, 50.0,
                std::numeric_limits<double>::infinity()
            ),
            std::invalid_argument
        );
    }
}

TEST_CASE("Invalid timestamps are rejected",
          "[deadline][validation]")
{
    REQUIRE_THROWS_AS(
        lms::DeadlineCalculator::calculate(
            "2026-15-42 29:90:00",
            "2026-11-07 23:59:00",
            85.0,
            100.0
        ),
        std::invalid_argument
    );
}
