
#ifndef LMS_ASSIGNMENT_H
#define LMS_ASSIGNMENT_H

#include <string>

namespace lms
{
    class Assignment
    {
    private:
        int assignmentId;
        int courseId;
        std::string title;
        std::string description;
        std::string deadline;
        double maxMark;

    public:
        Assignment(
            int assignmentId,
            int courseId,
            const std::string& title,
            const std::string& description,
            const std::string& deadline,
            double maxMark
        );

        int getAssignmentId() const;
        int getCourseId() const;
        std::string getTitle() const;
        std::string getDescription() const;
        std::string getDeadline() const;
        double getMaxMark() const;
    };
}

#endif
