
#ifndef LMS_SUBMISSION_REPOSITORY_H
#define LMS_SUBMISSION_REPOSITORY_H

#include "lms/Database.h"

#include <string>

namespace lms
{
    class SubmissionRepository
    {
    private:
        Database& database;

    public:
        explicit SubmissionRepository(Database& database);

        void submitAssignment(
            int studentId,
            int assignmentId,
            const std::string& filePath);
    };
}

#endif
