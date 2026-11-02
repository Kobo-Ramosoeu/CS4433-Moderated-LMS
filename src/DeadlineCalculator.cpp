
#include "lms/DeadlineCalculator.h"
#include "lms/Timestamp.h"

#include <algorithm>
#include <cmath>
#include <ctime>
#include <stdexcept>

namespace lms
{
    LatePenaltyResult DeadlineCalculator::calculate(
        const std::string& deadline,
        const std::string& submittedAt,
        double rawMark,
        double maxMark)
    {
        if (!std::isfinite(rawMark) ||
            !std::isfinite(maxMark) ||
            maxMark <= 0 ||
            rawMark < 0 ||
            rawMark > maxMark)
        {
            throw std::invalid_argument(
                "Marks must be valid and within the maximum mark.");
        }

        const std::time_t deadlineTime =
            parseLocalTimestamp(deadline, true);

        const std::time_t submittedTime =
            parseLocalTimestamp(submittedAt);

        const double seconds =
            std::difftime(submittedTime, deadlineTime);

        const int daysLate = seconds <= 0
            ? 0
            : static_cast<int>(seconds / 86400.0);

        double penalty = 0.0;
        double finalMark = rawMark;

        // Submissions more than three days late receive zero.
        if (seconds > 3 * 86400.0)
        {
            finalMark = 0.0;
            penalty = rawMark;
        }
        else if (daysLate > 0)
        {
            penalty = maxMark * 0.10 * daysLate;
            finalMark = std::max(0.0, rawMark - penalty);
        }

        return {daysLate, penalty, finalMark};
    }

    LatePenaltyResult DeadlineCalculator::calculateForStudent(
        DeadlineExtensionRepository& extensions,
        int assignmentId,
        int studentId,
        const std::string& submittedAt,
        double rawMark,
        double maxMark)
    {
        const std::string effectiveDeadline =
            extensions.getEffectiveDeadline(assignmentId, studentId);

        return calculate(
            effectiveDeadline,
            submittedAt,
            rawMark,
            maxMark);
    }
}
