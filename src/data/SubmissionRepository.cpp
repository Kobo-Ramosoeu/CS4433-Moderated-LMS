
#include "lms/SubmissionRepository.h"
#include "lms/DeadlineExtensionRepository.h"
#include "lms/Timestamp.h"

#include <sqlite3.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <filesystem>

namespace lms
{
    namespace
    {
        class ImmediateTransaction
        {
            Database& database;
            bool active = true;

        public:
            explicit ImmediateTransaction(Database& db)
                : database(db)
            {
                database.execute("BEGIN IMMEDIATE;");
            }

            void commit()
            {
                database.execute("COMMIT;");
                active = false;
            }

            ~ImmediateTransaction()
            {
                if (active)
                {
                    try
                    {
                        database.execute("ROLLBACK;");
                    }
                    catch (...)
                    {
                    }
                }
            }
        };
    }

    SubmissionRepository::SubmissionRepository(Database& database)
        : database(database)
    {
    }

    void SubmissionRepository::submitAssignment(
        int studentId,
        int assignmentId,
        const std::string& filePath)
    {
        if (filePath.empty())
        {
            throw std::invalid_argument(
                "File path cannot be empty.");
        }

        std::string cleanPath = filePath;

        // Remove surrounding quotation marks if provided.
        if (cleanPath.size() >= 2 &&
            cleanPath.front() == '"' &&
            cleanPath.back() == '"')
        {
            cleanPath = cleanPath.substr(
                1, cleanPath.size() - 2);
        }

        // Check that the file actually exists.
        std::error_code fileError;

        if (!std::filesystem::is_regular_file(
                std::filesystem::path(cleanPath), fileError))
        {
            throw std::invalid_argument(
                "The specified file does not exist or is not a regular file.");
        }

        // Confirm that the student is enrolled in the assignment's course.
        const char* permissionSql =
            "SELECT 1 "
            "FROM Assignment a "
            "JOIN Enrolment e ON e.course_id = a.course_id "
            "WHERE a.assignment_id = ? "
            "AND e.student_id = ?;";

        sqlite3_stmt* rawPermission = nullptr;

        if (sqlite3_prepare_v2(
                database.getConnection(),
                permissionSql,
                -1,
                &rawPermission,
                nullptr) != SQLITE_OK)
        {
            throw std::runtime_error(
                "Could not check assignment access.");
        }

        std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>
            permission(rawPermission, sqlite3_finalize);

        sqlite3_bind_int(permission.get(), 1, assignmentId);
        sqlite3_bind_int(permission.get(), 2, studentId);

        const int result = sqlite3_step(permission.get());

        if (result == SQLITE_DONE)
        {
            throw std::runtime_error(
                "Assignment not found or you are not enrolled "
                "in its course.");
        }

        if (result != SQLITE_ROW)
        {
            throw std::runtime_error(
                "Could not verify assignment access.");
        }

        // Serialize attempt allocation and insertion so concurrent
        // submissions cannot allocate the same attempt number.
        ImmediateTransaction transaction(database);

        // Determine the next attempt number for this student and assignment.
        const char* attemptSql =
            "SELECT COALESCE(MAX(attempt_number), 0) + 1 "
            "FROM Submission "
            "WHERE assignment_id = ? AND student_id = ?;";

        sqlite3_stmt* rawAttempt = nullptr;

        if (sqlite3_prepare_v2(
                database.getConnection(),
                attemptSql,
                -1,
                &rawAttempt,
                nullptr) != SQLITE_OK)
        {
            throw std::runtime_error(
                "Could not prepare attempt query.");
        }

        std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>
            attemptStatement(rawAttempt, sqlite3_finalize);

        sqlite3_bind_int(attemptStatement.get(), 1, assignmentId);
        sqlite3_bind_int(attemptStatement.get(), 2, studentId);

        if (sqlite3_step(attemptStatement.get()) != SQLITE_ROW)
        {
            throw std::runtime_error(
                "Could not determine submission attempt.");
        }

        const int attemptNumber =
            sqlite3_column_int(attemptStatement.get(), 0);

        // Get the effective deadline, including any approved extension.
        DeadlineExtensionRepository extensionRepository(database);

        const std::string effectiveDeadline =
            extensionRepository.getEffectiveDeadline(
                assignmentId, studentId);

        const std::time_t deadlineTime =
            parseLocalTimestamp(effectiveDeadline, true);

        // Capture the current time once so validation and persistence agree.
        const auto now = std::chrono::system_clock::now();

        const std::time_t timeNow =
            std::chrono::system_clock::to_time_t(now);

        const double secondsAfterDeadline =
            std::difftime(timeNow, deadlineTime);

        // Resubmissions are permitted on or before the effective deadline.
        if (attemptNumber > 1 && secondsAfterDeadline > 0)
        {
            throw std::runtime_error(
                "Resubmissions are allowed only on or before the effective deadline.");
        }

        // A first submission is accepted for up to 72 hours after the deadline.
        if (attemptNumber == 1 &&
            secondsAfterDeadline > 72.0 * 60.0 * 60.0)
        {
            throw std::runtime_error(
                "The 72-hour late submission window has closed.");
        }

        // Format the same captured time for database storage.
        std::tm localTime{};

#ifdef _WIN32
        if (localtime_s(&localTime, &timeNow) != 0)
        {
            throw std::runtime_error(
                "Could not determine current time.");
        }
#else
        if (localtime_r(&timeNow, &localTime) == nullptr)
        {
            throw std::runtime_error(
                "Could not determine current time.");
        }
#endif

        std::ostringstream timestamp;

        timestamp << std::put_time(
            &localTime, "%Y-%m-%d %H:%M:%S");

        const std::string submittedAt = timestamp.str();

        // Save the submission using a prepared statement.
        const char* insertSql =
            "INSERT INTO Submission "
            "(assignment_id, student_id, attempt_number, "
            "file_path, submitted_at) "
            "VALUES (?, ?, ?, ?, ?);";

        sqlite3_stmt* rawInsert = nullptr;

        if (sqlite3_prepare_v2(
                database.getConnection(),
                insertSql,
                -1,
                &rawInsert,
                nullptr) != SQLITE_OK)
        {
            throw std::runtime_error(
                "Could not prepare submission.");
        }

        std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>
            insertStatement(rawInsert, sqlite3_finalize);

        sqlite3_bind_int(insertStatement.get(), 1, assignmentId);
        sqlite3_bind_int(insertStatement.get(), 2, studentId);
        sqlite3_bind_int(insertStatement.get(), 3, attemptNumber);

        sqlite3_bind_text(
            insertStatement.get(),
            4,
            cleanPath.c_str(),
            -1,
            SQLITE_TRANSIENT);

        sqlite3_bind_text(
            insertStatement.get(),
            5,
            submittedAt.c_str(),
            -1,
            SQLITE_TRANSIENT);

        if (sqlite3_step(insertStatement.get()) != SQLITE_DONE)
        {
            throw std::runtime_error(
                "Could not save submission.");
        }

        // Commit only after the insertion succeeds.
        transaction.commit();

        std::cout << "\nSubmission saved successfully!\n";
        std::cout << "Attempt number: " << attemptNumber << '\n';
        std::cout << "Submitted at: " << submittedAt << '\n';
    }
}