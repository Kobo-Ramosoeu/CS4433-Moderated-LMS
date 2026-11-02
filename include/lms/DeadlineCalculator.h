
#ifndef LMS_DEADLINE_CALCULATOR_H
#define LMS_DEADLINE_CALCULATOR_H

#include <string>
#include "lms/DeadlineExtensionRepository.h"

namespace lms
{
    struct LatePenaltyResult
    {
        int daysLate;
        double penalty;
        double finalMark;
    };

    class DeadlineCalculator
    {
    public:
        static LatePenaltyResult calculate(
            const std::string& deadline,
            const std::string& submittedAt,
            double rawMark,
            double maxMark);

        static LatePenaltyResult calculateForStudent(
            DeadlineExtensionRepository& extensions,
            int assignmentId,
            int studentId,
            const std::string& submittedAt,
            double rawMark,
            double maxMark);
    };
}

#endif
