
#ifndef LMS_MODERATION_REPOSITORY_H
#define LMS_MODERATION_REPOSITORY_H

#include "lms/Database.h"

#include <string>
#include <vector>

namespace lms
{
    struct MarkForModeration
    {
        int markId;
        int submissionId;
        std::string assignmentTitle;
        std::string studentNumber;
        std::string lecturerName;
        double rawMark;
        double finalMark;
        std::string feedback;
        std::string status;
    };

    class ModerationRepository
    {
    private:
        Database& database;

    public:
        explicit ModerationRepository(Database& database);

        std::vector<MarkForModeration>
        getMarksAwaitingModeration(int hodId);

        std::vector<MarkForModeration> getApprovedMarksForRelease(int hodId);

        int releaseApprovedMarks(int hodId);

        void moderateMark(
            int hodId,
            int markId,
            const std::string& decision,
            const std::string& reason
        );

        void adjustMark(
            int hodId,
            int markId,
            double newRawMark,
            const std::string& reason
        );
    };
}

#endif
