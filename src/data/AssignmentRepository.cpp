#include "lms/AssignmentRepository.h"
#include "lms/Timestamp.h"

#include <sqlite3.h>

#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace lms
{
    AssignmentRepository::AssignmentRepository(Database& database)
        : database(database)
    {
    }

    std::vector<CourseOption>
    AssignmentRepository::getCoursesForLecturer(int lecturerId)
    {
        std::vector<CourseOption> courses;

        const char* sql =
            "SELECT c.course_id, c.course_code, c.course_name "
            "FROM Course c "
            "JOIN CourseLecturer cl ON c.course_id = cl.course_id "
            "WHERE cl.lecturer_id = ? "
            "ORDER BY c.course_code;";

        sqlite3_stmt* rawStatement = nullptr;

        if (sqlite3_prepare_v2(
                database.getConnection(),
                sql,
                -1,
                &rawStatement,
                nullptr) != SQLITE_OK)
        {
            throw std::runtime_error(
                "Could not prepare course query.");
        }

        std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>
            statement(rawStatement, sqlite3_finalize);

        sqlite3_bind_int(statement.get(), 1, lecturerId);

        int result;

        while ((result = sqlite3_step(statement.get())) == SQLITE_ROW)
        {
            CourseOption course;

            course.courseId =
                sqlite3_column_int(statement.get(), 0);

            const unsigned char* courseCode =
                sqlite3_column_text(statement.get(), 1);

            const unsigned char* courseName =
                sqlite3_column_text(statement.get(), 2);

            course.courseCode = courseCode
                ? reinterpret_cast<const char*>(courseCode)
                : "";

            course.courseName = courseName
                ? reinterpret_cast<const char*>(courseName)
                : "";

            courses.push_back(course);
        }

        if (result != SQLITE_DONE)
        {
            throw std::runtime_error(
                "Could not retrieve lecturer courses.");
        }

        return courses;
    }

    void AssignmentRepository::createAssignment(
        int lecturerId,
        int courseId,
        const std::string& title,
        const std::string& description,
        const std::string& deadline,
        double maxMark)
    {
        if (title.empty())
        {
            throw std::invalid_argument(
                "Assignment title cannot be empty.");
        }

        // Validate and standardize the deadline before saving it.
        const std::string normalizedDeadline =
            normalizeDeadline(deadline);

        // Reject zero, negative, NaN, and infinite maximum marks.
        if (!std::isfinite(maxMark) || maxMark <= 0)
        {
            throw std::invalid_argument(
                "Maximum mark must be a finite number greater than zero.");
        }

        // Confirm that this lecturer teaches the selected course.
        const char* permissionSql =
            "SELECT 1 FROM CourseLecturer "
            "WHERE lecturer_id = ? AND course_id = ?;";

        sqlite3_stmt* rawPermissionStatement = nullptr;

        if (sqlite3_prepare_v2(
                database.getConnection(),
                permissionSql,
                -1,
                &rawPermissionStatement,
                nullptr) != SQLITE_OK)
        {
            throw std::runtime_error(
                "Could not check lecturer course permissions.");
        }

        std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>
            permissionStatement(
                rawPermissionStatement, sqlite3_finalize);

        sqlite3_bind_int(permissionStatement.get(), 1, lecturerId);
        sqlite3_bind_int(permissionStatement.get(), 2, courseId);

        const int permissionResult =
            sqlite3_step(permissionStatement.get());

        if (permissionResult != SQLITE_ROW)
        {
            if (permissionResult == SQLITE_DONE)
            {
                throw std::runtime_error(
                    "You are not assigned to this course.");
            }

            throw std::runtime_error(
                "Could not verify lecturer permissions.");
        }

        const char* insertSql =
            "INSERT INTO Assignment "
            "(course_id, title, description, deadline, max_mark) "
            "VALUES (?, ?, ?, ?, ?);";

        sqlite3_stmt* rawInsertStatement = nullptr;

        if (sqlite3_prepare_v2(
                database.getConnection(),
                insertSql,
                -1,
                &rawInsertStatement,
                nullptr) != SQLITE_OK)
        {
            throw std::runtime_error(
                "Could not prepare assignment creation.");
        }

        std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>
            insertStatement(rawInsertStatement, sqlite3_finalize);

        sqlite3_bind_int(insertStatement.get(), 1, courseId);

        sqlite3_bind_text(
            insertStatement.get(),
            2,
            title.c_str(),
            -1,
            SQLITE_TRANSIENT);

        sqlite3_bind_text(
            insertStatement.get(),
            3,
            description.c_str(),
            -1,
            SQLITE_TRANSIENT);

        sqlite3_bind_text(
            insertStatement.get(),
            4,
            normalizedDeadline.c_str(),
            -1,
            SQLITE_TRANSIENT);

        sqlite3_bind_double(insertStatement.get(), 5, maxMark);

        if (sqlite3_step(insertStatement.get()) != SQLITE_DONE)
        {
            throw std::runtime_error(
                "Could not create assignment.");
        }
    }

    std::vector<Assignment>
    AssignmentRepository::getAssignmentsForLecturer(int lecturerId)
    {
        std::vector<Assignment> assignments;

        const char* sql =
            "SELECT a.assignment_id, a.course_id, a.title, "
            "a.description, a.deadline, a.max_mark "
            "FROM Assignment a "
            "JOIN CourseLecturer cl ON a.course_id = cl.course_id "
            "WHERE cl.lecturer_id = ? "
            "ORDER BY a.deadline;";

        sqlite3_stmt* rawStatement = nullptr;

        if (sqlite3_prepare_v2(
                database.getConnection(),
                sql,
                -1,
                &rawStatement,
                nullptr) != SQLITE_OK)
        {
            throw std::runtime_error(
                "Could not prepare assignment query.");
        }

        std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>
            statement(rawStatement, sqlite3_finalize);

        sqlite3_bind_int(statement.get(), 1, lecturerId);

        int result;

        while ((result = sqlite3_step(statement.get())) == SQLITE_ROW)
        {
            auto getText = [&](int column) -> std::string
            {
                const unsigned char* value =
                    sqlite3_column_text(statement.get(), column);

                return value
                    ? reinterpret_cast<const char*>(value)
                    : "";
            };

            assignments.emplace_back(
                sqlite3_column_int(statement.get(), 0),
                sqlite3_column_int(statement.get(), 1),
                getText(2),
                getText(3),
                getText(4),
                sqlite3_column_double(statement.get(), 5));
        }

        if (result != SQLITE_DONE)
        {
            throw std::runtime_error(
                "Could not retrieve assignments.");
        }

        return assignments;
    }

    std::vector<Assignment>
    AssignmentRepository::getAssignmentsForStudent(int studentId)
    {
        std::vector<Assignment> assignments;

        const char* sql =
            "SELECT a.assignment_id, a.course_id, a.title, "
            "a.description, a.deadline, a.max_mark "
            "FROM Assignment a "
            "JOIN Enrolment e ON a.course_id = e.course_id "
            "WHERE e.student_id = ? "
            "ORDER BY a.deadline;";

        sqlite3_stmt* rawStatement = nullptr;

        if (sqlite3_prepare_v2(
                database.getConnection(),
                sql,
                -1,
                &rawStatement,
                nullptr) != SQLITE_OK)
        {
            throw std::runtime_error(
                "Could not prepare student assignment query.");
        }

        std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>
            statement(rawStatement, sqlite3_finalize);

        sqlite3_bind_int(statement.get(), 1, studentId);

        int result;

        while ((result = sqlite3_step(statement.get())) == SQLITE_ROW)
        {
            auto getText = [&](int column) -> std::string
            {
                const unsigned char* value =
                    sqlite3_column_text(statement.get(), column);

                return value
                    ? reinterpret_cast<const char*>(value)
                    : "";
            };

            assignments.emplace_back(
                sqlite3_column_int(statement.get(), 0),
                sqlite3_column_int(statement.get(), 1),
                getText(2),
                getText(3),
                getText(4),
                sqlite3_column_double(statement.get(), 5));
        }

        if (result != SQLITE_DONE)
        {
            throw std::runtime_error(
                "Could not retrieve student assignments.");
        }

        return assignments;
    }
}