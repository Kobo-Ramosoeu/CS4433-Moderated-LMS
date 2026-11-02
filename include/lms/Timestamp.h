#ifndef LMS_TIMESTAMP_H
#define LMS_TIMESTAMP_H

#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

namespace lms {

// Parse local timestamps. Legacy date-only deadlines retain their original
// end-of-day meaning; submission timestamps must include a time.
inline std::time_t parseLocalTimestamp(const std::string& value,
                                       bool allowLegacyDateOnly = false) {
    std::tm parsed{};
    std::istringstream input(value);

    if (value.size() == 10 && allowLegacyDateOnly) {
        input >> std::get_time(&parsed, "%Y-%m-%d");
        parsed.tm_hour = 23;
        parsed.tm_min = 59;
        parsed.tm_sec = 59;
    } else if (value.size() == 19) {
        input >> std::get_time(&parsed, "%Y-%m-%d %H:%M:%S");
    } else if (value.size() == 16) {
        input >> std::get_time(&parsed, "%Y-%m-%d %H:%M");
        parsed.tm_sec = 0;
    } else {
        throw std::invalid_argument("Use YYYY-MM-DD HH:MM:SS (legacy deadlines may use YYYY-MM-DD).");
    }

    if (input.fail()) throw std::invalid_argument("Invalid date/time format.");
    parsed.tm_isdst = -1;
    const std::tm requested = parsed;
    const std::time_t result = std::mktime(&parsed);
    if (result == static_cast<std::time_t>(-1))
        throw std::invalid_argument("Date/time is outside the supported range.");

    std::tm checked{};
#ifdef _WIN32
    if (localtime_s(&checked, &result) != 0)
        throw std::invalid_argument("Could not validate date/time.");
#else
    if (localtime_r(&result, &checked) == nullptr)
        throw std::invalid_argument("Could not validate date/time.");
#endif
    if (checked.tm_year != requested.tm_year || checked.tm_mon != requested.tm_mon ||
        checked.tm_mday != requested.tm_mday || checked.tm_hour != requested.tm_hour ||
        checked.tm_min != requested.tm_min || checked.tm_sec != requested.tm_sec)
        throw std::invalid_argument("Invalid calendar date or time.");
    return result;
}

inline std::string normalizeDeadline(const std::string& value) {
    const std::time_t parsed = parseLocalTimestamp(value, true);
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &parsed);
#else
    localtime_r(&parsed, &local);
#endif
    std::ostringstream output;
    output << std::put_time(&local, "%Y-%m-%d %H:%M:%S");
    return output.str();
}

} // namespace lms
#endif
