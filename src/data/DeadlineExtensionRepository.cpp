
#include "lms/DeadlineExtensionRepository.h"
#include "lms/Timestamp.h"

#include <sqlite3.h>

#include <memory>
#include <stdexcept>

namespace lms
{
    DeadlineExtensionRepository::DeadlineExtensionRepository(
        Database& database)
        : database(database)
    {
    }

    void DeadlineExtensionRepository::grantExtension(
        int lecturerId,
        int assignmentId,
        int studentId,
        const std::string& newDeadline,
        const std::string& reason)
    {
        if (lecturerId <= 0 || assignmentId <= 0 ||
            studentId < 0 || reason.empty())
        {
            throw std::invalid_argument(
                "Invalid deadline extension details.");
        }

        const std::string normalizedDeadline =
            normalizeDeadline(newDeadline);

        const char* sql =
            "INSERT INTO DeadlineExtension "
            "(assignment_id, student_id, new_deadline, reason, granted_by) "
            "SELECT ?, ?, ?, ?, ? "
            "WHERE EXISTS ("
            " SELECT 1 FROM CourseLecturer cl "
            " JOIN Assignment a ON a.course_id = cl.course_id "
            " WHERE a.assignment_id = ? AND cl.lecturer_id = ?"
            ") "
            "AND (? = 0 OR EXISTS ("
            " SELECT 1 FROM Enrolment e "
            " JOIN Assignment a ON a.course_id = e.course_id "
            " WHERE a.assignment_id = ? AND e.student_id = ?"
            "));";

        sqlite3_stmt* raw = nullptr;

        if (sqlite3_prepare_v2(
                database.getConnection(), sql, -1, &raw, nullptr)
            != SQLITE_OK)
        {
            throw std::runtime_error(
                "Could not prepare deadline extension.");
        }

        std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>
            statement(raw, sqlite3_finalize);

        sqlite3_bind_int(statement.get(), 1, assignmentId);

        if (studentId == 0)
            sqlite3_bind_null(statement.get(), 2);
        else
            sqlite3_bind_int(statement.get(), 2, studentId);

        sqlite3_bind_text(
            statement.get(), 3, normalizedDeadline.c_str(),
            -1, SQLITE_TRANSIENT);

        sqlite3_bind_text(
            statement.get(), 4, reason.c_str(),
            -1, SQLITE_TRANSIENT);

        sqlite3_bind_int(statement.get(), 5, lecturerId);
        sqlite3_bind_int(statement.get(), 6, assignmentId);
        sqlite3_bind_int(statement.get(), 7, lecturerId);
        sqlite3_bind_int(statement.get(), 8, studentId);
        sqlite3_bind_int(statement.get(), 9, assignmentId);
        sqlite3_bind_int(statement.get(), 10, studentId);

        if (sqlite3_step(statement.get()) != SQLITE_DONE)
        {
            throw std::runtime_error(
                "Could not save deadline extension.");
        }

        if (sqlite3_changes(database.getConnection()) == 0)
        {
            throw std::runtime_error(
                "Extension not saved. Check the assignment, "
                "student enrolment, and lecturer permissions.");
        }
    }

    std::string DeadlineExtensionRepository::getEffectiveDeadline(
        int assignmentId,
        int studentId)
    {
        const char* sql =
            "SELECT COALESCE("
            " (SELECT MAX(de.new_deadline) "
            "  FROM DeadlineExtension de "
            "  WHERE de.assignment_id = a.assignment_id "
            "    AND (de.student_id = ? OR de.student_id IS NULL)), "
            " a.deadline) "
            "FROM Assignment a "
            "WHERE a.assignment_id = ?;";

        sqlite3_stmt* raw = nullptr;

        if (sqlite3_prepare_v2(
                database.getConnection(), sql, -1, &raw, nullptr)
            != SQLITE_OK)
        {
            throw std::runtime_error(
                "Could not prepare effective deadline query.");
        }

        std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>
            statement(raw, sqlite3_finalize);

        sqlite3_bind_int(statement.get(), 1, studentId);
        sqlite3_bind_int(statement.get(), 2, assignmentId);

        if (sqlite3_step(statement.get()) != SQLITE_ROW)
        {
            throw std::runtime_error(
                "Assignment not found.");
        }

        const unsigned char* value =
            sqlite3_column_text(statement.get(), 0);

        return value
            ? reinterpret_cast<const char*>(value)
            : "";
    }
}
