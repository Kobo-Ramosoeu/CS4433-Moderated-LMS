
#ifndef LMS_ASSIGNMENT_REPOSITORY_H
#define LMS_ASSIGNMENT_REPOSITORY_H

#include "lms/Database.h"
#include "lms/models/Assignment.h"

#include <string>
#include <vector>

namespace lms
{
    struct CourseOption
    {
        int courseId;
        std::string courseCode;
        std::string courseName;
    };

    class AssignmentRepository
    {
    private:
        Database& database;

    public:
        explicit AssignmentRepository(Database& database);

        std::vector<CourseOption> getCoursesForLecturer(
            int lecturerId);

        void createAssignment(
            int lecturerId,
            int courseId,
            const std::string& title,
            const std::string& description,
            const std::string& deadline,
            double maxMark);

        std::vector<Assignment> getAssignmentsForLecturer(int lecturerId);
        std::vector<Assignment> getAssignmentsForStudent(int studentId);
    };
}

#endif
