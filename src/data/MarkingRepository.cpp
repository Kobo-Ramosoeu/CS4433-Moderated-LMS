
#include "lms/MarkingRepository.h"

#include "lms/DeadlineCalculator.h"
#include "lms/DeadlineExtensionRepository.h"

#include <sqlite3.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <cmath>

namespace lms
{
    namespace
    {
        using Statement = std::unique_ptr<
            sqlite3_stmt, decltype(&sqlite3_finalize)>;

        Statement prepare(sqlite3* db, const char* sql)
        {
            sqlite3_stmt* raw = nullptr;

            if (sqlite3_prepare_v2(
                    db, sql, -1, &raw, nullptr) != SQLITE_OK)
            {
                throw std::runtime_error(
                    "Could not prepare marking database query.");
            }

            return Statement(raw, sqlite3_finalize);
        }

        std::string currentTimestamp()
        {
            const auto now = std::chrono::system_clock::now();
            const std::time_t timeNow =
                std::chrono::system_clock::to_time_t(now);

            std::tm local{};

#ifdef _WIN32
            if (localtime_s(&local, &timeNow) != 0)
#else
            if (localtime_r(&timeNow, &local) == nullptr)
#endif
            {
                throw std::runtime_error(
                    "Could not determine current time.");
            }

            std::ostringstream output;
            output << std::put_time(&local, "%Y-%m-%d %H:%M:%S");
            return output.str();
        }

        std::string columnText(sqlite3_stmt* statement, int column)
        {
            const auto* value = sqlite3_column_text(statement, column);

            return value
                ? reinterpret_cast<const char*>(value)
                : "";
        }
    }

    MarkingRepository::MarkingRepository(Database& database)
        : database(database)
    {
    }

    std::vector<SubmissionToMark>
    MarkingRepository::getSubmissionsForLecturer(int lecturerId)
    {
        const char* sql =
            "SELECT s.submission_id, s.assignment_id, s.student_id, "
            "s.attempt_number, a.title, st.student_number, "
            "s.submitted_at, a.max_mark "
            "FROM Submission s "
            "JOIN Assignment a ON a.assignment_id = s.assignment_id "
            "JOIN CourseLecturer cl ON cl.course_id = a.course_id "
            "JOIN Student st ON st.user_id = s.student_id "
            "LEFT JOIN Mark m ON m.submission_id = s.submission_id "
            "WHERE cl.lecturer_id = ? AND m.mark_id IS NULL "
            "ORDER BY s.submitted_at;";

        auto statement = prepare(database.getConnection(), sql);
        sqlite3_bind_int(statement.get(), 1, lecturerId);

        std::vector<SubmissionToMark> submissions;
        int result;

        while ((result = sqlite3_step(statement.get())) == SQLITE_ROW)
        {
            submissions.push_back({
                sqlite3_column_int(statement.get(), 0),
                sqlite3_column_int(statement.get(), 1),
                sqlite3_column_int(statement.get(), 2),
                sqlite3_column_int(statement.get(), 3),
                columnText(statement.get(), 4),
                columnText(statement.get(), 5),
                columnText(statement.get(), 6),
                sqlite3_column_double(statement.get(), 7)
            });
        }

        if (result != SQLITE_DONE)
        {
            throw std::runtime_error(
                "Could not retrieve submissions for marking.");
        }

        return submissions;
    }

    MarkingResult MarkingRepository::enterMark(
        int lecturerId,
        int submissionId,
        double rawMark,
        const std::string& feedback)
    {
        if (lecturerId <= 0 || submissionId <= 0 ||
            feedback.empty() || !std::isfinite(rawMark) ||
            rawMark < 0)
        {
            throw std::invalid_argument(
                "Please provide a valid mark and feedback.");
        }

        sqlite3* db = database.getConnection();

        // Retrieve the submission and verify the lecturer's course access.
        const char* detailsSql =
            "SELECT s.assignment_id, s.student_id, s.submitted_at, "
            "a.max_mark "
            "FROM Submission s "
            "JOIN Assignment a ON a.assignment_id = s.assignment_id "
            "JOIN CourseLecturer cl ON cl.course_id = a.course_id "
            "WHERE s.submission_id = ? AND cl.lecturer_id = ?;";

        auto details = prepare(db, detailsSql);
        sqlite3_bind_int(details.get(), 1, submissionId);
        sqlite3_bind_int(details.get(), 2, lecturerId);

        if (sqlite3_step(details.get()) != SQLITE_ROW)
        {
            throw std::runtime_error(
                "Submission not found or you are not its assigned lecturer.");
        }

        const int assignmentId =
            sqlite3_column_int(details.get(), 0);

        const int studentId =
            sqlite3_column_int(details.get(), 1);

        const std::string submittedAt =
            columnText(details.get(), 2);

        const double maxMark =
            sqlite3_column_double(details.get(), 3);

        if (rawMark > maxMark)
        {
            throw std::invalid_argument(
                "Raw mark cannot exceed the assignment's maximum mark.");
        }

        // Use the shared repository to resolve the effective deadline.
        DeadlineExtensionRepository extensions(database);

        const LatePenaltyResult penalty =
            DeadlineCalculator::calculateForStudent(
                extensions,
                assignmentId,
                studentId,
                submittedAt,
                rawMark,
                maxMark);

        const std::string markedAt = currentTimestamp();

        // Save the mark and audit record in one transaction.
        char* errorMessage = nullptr;

        if (sqlite3_exec(db, "BEGIN IMMEDIATE;", nullptr, nullptr,
                         &errorMessage) != SQLITE_OK)
        {
            std::string error = errorMessage
                ? errorMessage : "Could not start transaction.";

            sqlite3_free(errorMessage);
            throw std::runtime_error(error);
        }

        try
        {
            const char* insertMarkSql =
                "INSERT INTO Mark "
                "(submission_id, raw_mark, final_mark, feedback, "
                "status, marked_by, marked_at) "
                "VALUES (?, ?, ?, ?, 'AwaitingModeration', ?, ?);";

            auto insertMark = prepare(db, insertMarkSql);

            sqlite3_bind_int(insertMark.get(), 1, submissionId);
            sqlite3_bind_double(insertMark.get(), 2, rawMark);
            sqlite3_bind_double(insertMark.get(), 3, penalty.finalMark);

            sqlite3_bind_text(
                insertMark.get(), 4, feedback.c_str(),
                -1, SQLITE_TRANSIENT);

            sqlite3_bind_int(insertMark.get(), 5, lecturerId);

            sqlite3_bind_text(
                insertMark.get(), 6, markedAt.c_str(),
                -1, SQLITE_TRANSIENT);

            if (sqlite3_step(insertMark.get()) != SQLITE_DONE)
            {
                throw std::runtime_error(
                    "Could not save mark. It may already be marked.");
            }

            const sqlite3_int64 markId =
                sqlite3_last_insert_rowid(db);

            std::ostringstream oldValue;
            oldValue << "No previous mark";

            std::ostringstream newValue;
            newValue << "raw=" << rawMark
                     << "; final=" << penalty.finalMark;

            const char* auditSql =
                "INSERT INTO AuditRecord "
                "(user_id, mark_id, action, old_value, new_value, "
                "reason, action_at) "
                "VALUES (?, ?, 'MarkEntered', ?, ?, ?, ?);";

            auto audit = prepare(db, auditSql);

            sqlite3_bind_int(audit.get(), 1, lecturerId);
            sqlite3_bind_int64(audit.get(), 2, markId);

            const std::string oldText = oldValue.str();
            const std::string newText = newValue.str();

            sqlite3_bind_text(
                audit.get(), 3, oldText.c_str(),
                -1, SQLITE_TRANSIENT);

            sqlite3_bind_text(
                audit.get(), 4, newText.c_str(),
                -1, SQLITE_TRANSIENT);

            sqlite3_bind_text(
                audit.get(), 5, feedback.c_str(),
                -1, SQLITE_TRANSIENT);

            sqlite3_bind_text(
                audit.get(), 6, markedAt.c_str(),
                -1, SQLITE_TRANSIENT);

            if (sqlite3_step(audit.get()) != SQLITE_DONE)
            {
                throw std::runtime_error(
                    "Could not save mark audit record.");
            }

            if (sqlite3_exec(db, "COMMIT;", nullptr, nullptr,
                             &errorMessage) != SQLITE_OK)
            {
                std::string error = errorMessage
                    ? errorMessage : "Could not commit mark.";

                sqlite3_free(errorMessage);
                errorMessage = nullptr;

                throw std::runtime_error(error);
            }
        }
        catch (...)
        {
            sqlite3_exec(
                db, "ROLLBACK;", nullptr, nullptr, nullptr);

            throw;
        }

        return {
            penalty.daysLate,
            rawMark,
            penalty.penalty,
            penalty.finalMark
        };
    }

    std::vector<ReturnedMark>
    MarkingRepository::getReturnedMarksForLecturer(int lecturerId)
    {
        const char* sql =
            "SELECT m.mark_id, s.submission_id, a.title, st.student_number, "
            "m.raw_mark, m.final_mark, a.max_mark, m.feedback, "
            "COALESCE((SELECT mr.reason FROM ModerationRecord mr "
            " WHERE mr.mark_id = m.mark_id "
            " AND mr.decision = 'CorrectionRequired' "
            " ORDER BY mr.action_at DESC, mr.moderation_id DESC LIMIT 1), '') "
            "FROM Mark m "
            "JOIN Submission s ON s.submission_id = m.submission_id "
            "JOIN Assignment a ON a.assignment_id = s.assignment_id "
            "JOIN Student st ON st.user_id = s.student_id "
            "JOIN CourseLecturer cl ON cl.course_id = a.course_id "
            "WHERE cl.lecturer_id = ? AND m.marked_by = ? "
            "AND m.status = 'CorrectionRequired' "
            "ORDER BY m.mark_id;";

        auto statement = prepare(database.getConnection(), sql);

        sqlite3_bind_int(statement.get(), 1, lecturerId);
        sqlite3_bind_int(statement.get(), 2, lecturerId);

        std::vector<ReturnedMark> marks;
        int result;

        while ((result = sqlite3_step(statement.get())) == SQLITE_ROW)
        {
            marks.push_back({
                sqlite3_column_int(statement.get(), 0),
                sqlite3_column_int(statement.get(), 1),
                columnText(statement.get(), 2),
                columnText(statement.get(), 3),
                sqlite3_column_double(statement.get(), 4),
                sqlite3_column_double(statement.get(), 5),
                sqlite3_column_double(statement.get(), 6),
                columnText(statement.get(), 7),
                columnText(statement.get(), 8)
            });
        }

        if (result != SQLITE_DONE)
        {
            throw std::runtime_error(
                "Could not retrieve returned marks.");
        }

        return marks;
    }

    MarkingResult MarkingRepository::correctReturnedMark(
        int lecturerId,
        int markId,
        double rawMark,
        const std::string& feedback)
    {
        if (lecturerId <= 0 || markId <= 0 ||
            feedback.empty() || !std::isfinite(rawMark) ||
            rawMark < 0)
        {
            throw std::invalid_argument(
                "Please provide a valid mark and feedback.");
        }

        sqlite3* db = database.getConnection();

        const char* detailsSql =
            "SELECT s.assignment_id, s.student_id, "
            "s.submitted_at, a.max_mark "
            "FROM Mark m "
            "JOIN Submission s ON s.submission_id = m.submission_id "
            "JOIN Assignment a ON a.assignment_id = s.assignment_id "
            "JOIN CourseLecturer cl ON cl.course_id = a.course_id "
            "WHERE m.mark_id = ? "
            "AND m.marked_by = ? "
            "AND cl.lecturer_id = ? "
            "AND m.status = 'CorrectionRequired';";

        auto details = prepare(db, detailsSql);

        sqlite3_bind_int(details.get(), 1, markId);
        sqlite3_bind_int(details.get(), 2, lecturerId);
        sqlite3_bind_int(details.get(), 3, lecturerId);

        if (sqlite3_step(details.get()) != SQLITE_ROW)
        {
            throw std::runtime_error(
                "Returned mark not found or not assigned to you.");
        }

        const int assignmentId =
            sqlite3_column_int(details.get(), 0);

        const int studentId =
            sqlite3_column_int(details.get(), 1);

        const std::string submittedAt =
            columnText(details.get(), 2);

        const double maxMark =
            sqlite3_column_double(details.get(), 3);

        if (rawMark > maxMark)
        {
            throw std::invalid_argument(
                "Raw mark cannot exceed the assignment's maximum mark.");
        }

        // Use the same effective-deadline calculation as initial marking.
        DeadlineExtensionRepository extensions(database);

        const LatePenaltyResult penalty =
            DeadlineCalculator::calculateForStudent(
                extensions,
                assignmentId,
                studentId,
                submittedAt,
                rawMark,
                maxMark);

        const std::string now = currentTimestamp();

        char* errorMessage = nullptr;

        if (sqlite3_exec(db, "BEGIN IMMEDIATE;", nullptr, nullptr,
                         &errorMessage) != SQLITE_OK)
        {
            std::string error = errorMessage
                ? errorMessage : "Could not start transaction.";

            sqlite3_free(errorMessage);
            throw std::runtime_error(error);
        }

        try
        {
            auto oldStmt = prepare(
                db,
                "SELECT raw_mark, final_mark, feedback "
                "FROM Mark WHERE mark_id = ?;");

            sqlite3_bind_int(oldStmt.get(), 1, markId);

            if (sqlite3_step(oldStmt.get()) != SQLITE_ROW)
            {
                throw std::runtime_error(
                    "Could not retrieve previous mark values.");
            }

            std::ostringstream oldValue;

            oldValue << "raw="
                     << sqlite3_column_double(oldStmt.get(), 0)
                     << "; final="
                     << sqlite3_column_double(oldStmt.get(), 1)
                     << "; feedback="
                     << columnText(oldStmt.get(), 2);

            auto update = prepare(
                db,
                "UPDATE Mark SET raw_mark = ?, final_mark = ?, "
                "feedback = ?, status = 'AwaitingModeration', "
                "marked_at = ? "
                "WHERE mark_id = ? AND marked_by = ? "
                "AND status = 'CorrectionRequired';");

            sqlite3_bind_double(update.get(), 1, rawMark);
            sqlite3_bind_double(update.get(), 2, penalty.finalMark);

            sqlite3_bind_text(
                update.get(), 3, feedback.c_str(),
                -1, SQLITE_TRANSIENT);

            sqlite3_bind_text(
                update.get(), 4, now.c_str(),
                -1, SQLITE_TRANSIENT);

            sqlite3_bind_int(update.get(), 5, markId);
            sqlite3_bind_int(update.get(), 6, lecturerId);

            if (sqlite3_step(update.get()) != SQLITE_DONE ||
                sqlite3_changes(db) != 1)
            {
                throw std::runtime_error(
                    "Could not update returned mark.");
            }

            std::ostringstream newValue;

            newValue << "raw=" << rawMark
                     << "; final=" << penalty.finalMark
                     << "; feedback=" << feedback;

            auto audit = prepare(
                db,
                "INSERT INTO AuditRecord "
                "(user_id, mark_id, action, old_value, new_value, "
                "reason, action_at) "
                "VALUES (?, ?, 'MarkCorrected', ?, ?, ?, ?);");

            sqlite3_bind_int(audit.get(), 1, lecturerId);
            sqlite3_bind_int(audit.get(), 2, markId);

            const std::string oldText = oldValue.str();
            const std::string newText = newValue.str();

            sqlite3_bind_text(
                audit.get(), 3, oldText.c_str(),
                -1, SQLITE_TRANSIENT);

            sqlite3_bind_text(
                audit.get(), 4, newText.c_str(),
                -1, SQLITE_TRANSIENT);

            sqlite3_bind_text(
                audit.get(), 5,
                "Lecturer corrected returned mark",
                -1, SQLITE_STATIC);

            sqlite3_bind_text(
                audit.get(), 6, now.c_str(),
                -1, SQLITE_TRANSIENT);

            if (sqlite3_step(audit.get()) != SQLITE_DONE)
            {
                throw std::runtime_error(
                    "Could not save correction audit record.");
            }

            if (sqlite3_exec(db, "COMMIT;", nullptr, nullptr,
                             &errorMessage) != SQLITE_OK)
            {
                std::string error = errorMessage
                    ? errorMessage : "Could not commit correction.";

                sqlite3_free(errorMessage);
                errorMessage = nullptr;

                throw std::runtime_error(error);
            }
        }
        catch (...)
        {
            sqlite3_exec(
                db, "ROLLBACK;", nullptr, nullptr, nullptr);

            throw;
        }

        return {
            penalty.daysLate,
            rawMark,
            penalty.penalty,
            penalty.finalMark
        };
    }

    std::vector<StudentReleasedMark>
    MarkingRepository::getReleasedMarksForStudent(int studentId)
    {
        if (studentId <= 0)
        {
            throw std::invalid_argument("Invalid student ID.");
        }

        const char* sql = R"(
            SELECT a.title, c.course_code, m.raw_mark, m.final_mark,
                   a.max_mark, m.feedback, m.marked_at
            FROM Mark m
            JOIN Submission s ON s.submission_id = m.submission_id
            JOIN Assignment a ON a.assignment_id = s.assignment_id
            JOIN Course c ON c.course_id = a.course_id
            WHERE s.student_id = ? AND m.status = 'Released'
            ORDER BY m.marked_at DESC, m.mark_id DESC
        )";

        auto statement = prepare(database.getConnection(), sql);
        sqlite3_bind_int(statement.get(), 1, studentId);

        std::vector<StudentReleasedMark> marks;
        int result;

        while ((result = sqlite3_step(statement.get())) == SQLITE_ROW)
        {
            marks.push_back({
                columnText(statement.get(), 0),
                columnText(statement.get(), 1),
                sqlite3_column_double(statement.get(), 2),
                sqlite3_column_double(statement.get(), 3),
                sqlite3_column_double(statement.get(), 4),
                columnText(statement.get(), 5),
                columnText(statement.get(), 6)
            });
        }

        if (result != SQLITE_DONE)
        {
            throw std::runtime_error(
                "Could not retrieve released marks.");
        }

        return marks;
    }
}
