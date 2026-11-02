
#ifndef LMS_DEADLINE_EXTENSION_REPOSITORY_H
#define LMS_DEADLINE_EXTENSION_REPOSITORY_H

#include "lms/Database.h"

#include <string>

namespace lms
{
    class DeadlineExtensionRepository
    {
    private:
        Database& database;

    public:
        explicit DeadlineExtensionRepository(Database& database);

        void grantExtension(
            int lecturerId,
            int assignmentId,
            int studentId,
            const std::string& newDeadline,
            const std::string& reason);

        std::string getEffectiveDeadline(
            int assignmentId,
            int studentId);
    };
}

#endif
