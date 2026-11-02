
#include "lms/ModerationRepository.h"
#include "lms/DeadlineCalculator.h"
#include "lms/DeadlineExtensionRepository.h"

#include <sqlite3.h>

#include <ctime>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace lms
{
    namespace
    {
        std::string currentTimestamp()
        {
            const std::time_t now = std::time(nullptr);
            std::tm localTime{};

#ifdef _WIN32
            localtime_s(&localTime, &now);
#else
            localtime_r(&now, &localTime);
#endif

            std::ostringstream output;
            output << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
            return output.str();
        }

        class Statement
        {
        private:
            sqlite3_stmt* statement = nullptr;

        public:
            Statement(sqlite3* db, const char* sql)
            {
                if (sqlite3_prepare_v2(db, sql, -1, &statement, nullptr)
                    != SQLITE_OK)
                {
                    throw std::runtime_error(sqlite3_errmsg(db));
                }
            }

            ~Statement()
            {
                sqlite3_finalize(statement);
            }

            Statement(const Statement&) = delete;
            Statement& operator=(const Statement&) = delete;

            sqlite3_stmt* get() const
            {
                return statement;
            }
        };

        void bindText(
            sqlite3* db,
            sqlite3_stmt* statement,
            int index,
            const std::string& value)
        {
            if (sqlite3_bind_text(
                    statement, index, value.c_str(),
                    -1, SQLITE_TRANSIENT) != SQLITE_OK)
            {
                throw std::runtime_error(sqlite3_errmsg(db));
            }
        }

        void bindInt(
            sqlite3* db,
            sqlite3_stmt* statement,
            int index,
            int value)
        {
            if (sqlite3_bind_int(statement, index, value) != SQLITE_OK)
            {
                throw std::runtime_error(sqlite3_errmsg(db));
            }
        }

        void execute(sqlite3* db, const char* sql)
        {
            char* error = nullptr;

            if (sqlite3_exec(db, sql, nullptr, nullptr, &error) != SQLITE_OK)
            {
                std::string message = error ? error : sqlite3_errmsg(db);
                sqlite3_free(error);
                throw std::runtime_error(message);
            }
        }

        bool hodOwnsCourse(
            sqlite3* db,
            int hodId,
            int markId)
        {
            Statement query(db, R"(
                SELECT 1
                FROM Mark m
                JOIN Submission s ON s.submission_id = m.submission_id
                JOIN Assignment a ON a.assignment_id = s.assignment_id
                JOIN Course c ON c.course_id = a.course_id
                JOIN Department d ON d.department_id = c.department_id
                JOIN HeadOfDepartment h ON h.user_id = d.hod_user_id
                WHERE h.user_id = ?
                  AND m.mark_id = ?
                LIMIT 1
            )");

            bindInt(db, query.get(), 1, hodId);
            bindInt(db, query.get(), 2, markId);

            return sqlite3_step(query.get()) == SQLITE_ROW;
        }
    }

    ModerationRepository::ModerationRepository(Database& database)
        : database(database)
    {
    }

    std::vector<MarkForModeration>
    ModerationRepository::getMarksAwaitingModeration(int hodId)
    {
        if (hodId <= 0)
        {
            throw std::invalid_argument("Invalid HoD user ID.");
        }

        sqlite3* db = database.getConnection();

        Statement query(db, R"(
            SELECT
                m.mark_id,
                m.submission_id,
                a.title,
                st.student_number,
                u.full_name,
                m.raw_mark,
                m.final_mark,
                m.feedback,
                m.status
            FROM Mark m
            JOIN Submission s ON s.submission_id = m.submission_id
            JOIN Assignment a ON a.assignment_id = s.assignment_id
            JOIN Course c ON c.course_id = a.course_id
            JOIN Department d ON d.department_id = c.department_id
            JOIN HeadOfDepartment h ON h.user_id = d.hod_user_id
            JOIN Student st ON st.user_id = s.student_id
            JOIN Lecturer l ON l.user_id = m.marked_by
            JOIN "User" u ON u.user_id = l.user_id
            WHERE h.user_id = ?
              AND m.status = 'AwaitingModeration'
            ORDER BY m.marked_at, m.mark_id
        )");

        bindInt(db, query.get(), 1, hodId);

        std::vector<MarkForModeration> results;
        int stepResult;

        while ((stepResult = sqlite3_step(query.get())) == SQLITE_ROW)
        {
            auto columnText = [&](int index) -> std::string
            {
                const auto* value = sqlite3_column_text(query.get(), index);
                return value
                    ? reinterpret_cast<const char*>(value)
                    : "";
            };

            MarkForModeration mark{};
            mark.markId = sqlite3_column_int(query.get(), 0);
            mark.submissionId = sqlite3_column_int(query.get(), 1);
            mark.assignmentTitle = columnText(2);
            mark.studentNumber = columnText(3);
            mark.lecturerName = columnText(4);
            mark.rawMark = sqlite3_column_double(query.get(), 5);
            mark.finalMark = sqlite3_column_double(query.get(), 6);
            mark.feedback = columnText(7);
            mark.status = columnText(8);

            results.push_back(mark);
        }

        if (stepResult != SQLITE_DONE)
        {
            throw std::runtime_error(sqlite3_errmsg(db));
        }

        return results;
    }

    void ModerationRepository::moderateMark(
        int hodId,
        int markId,
        const std::string& decision,
        const std::string& reason)
    {
        if (hodId <= 0 || markId <= 0)
        {
            throw std::invalid_argument("Invalid HoD ID or mark ID.");
        }

        if (decision != "Approved" &&
            decision != "CorrectionRequired")
        {
            throw std::invalid_argument("Invalid moderation decision.");
        }

        if (decision == "CorrectionRequired" && reason.empty())
        {
            throw std::invalid_argument(
                "A reason is required when returning a mark for correction.");
        }

        sqlite3* db = database.getConnection();
        bool transactionStarted = false;

        try
        {
            execute(db, "BEGIN IMMEDIATE;");
            transactionStarted = true;

            Statement checkHod(db, R"(
                SELECT 1
                FROM HeadOfDepartment
                WHERE user_id = ?
            )");

            bindInt(db, checkHod.get(), 1, hodId);

            if (sqlite3_step(checkHod.get()) != SQLITE_ROW)
            {
                throw std::runtime_error(
                    "This user is not registered as a Head of Department.");
            }

            if (!hodOwnsCourse(db, hodId, markId))
            {
                throw std::runtime_error(
                    "This mark does not belong to your department.");
            }

            Statement checkStatus(db, R"(
                SELECT status, raw_mark, final_mark
                FROM Mark
                WHERE mark_id = ?
            )");

            bindInt(db, checkStatus.get(), 1, markId);

            if (sqlite3_step(checkStatus.get()) != SQLITE_ROW)
            {
                throw std::runtime_error("Mark not found.");
            }

            const auto* statusText =
                sqlite3_column_text(checkStatus.get(), 0);

            const std::string oldStatus = statusText
                ? reinterpret_cast<const char*>(statusText)
                : "";

            if (oldStatus != "AwaitingModeration")
            {
                throw std::runtime_error(
                    "Only marks awaiting moderation can be reviewed.");
            }

            const double rawMark =
                sqlite3_column_double(checkStatus.get(), 1);
            const double finalMark =
                sqlite3_column_double(checkStatus.get(), 2);

            Statement update(db, R"(
                UPDATE Mark
                SET status = ?
                WHERE mark_id = ?
                  AND status = 'AwaitingModeration'
            )");

            bindText(db, update.get(), 1, decision);
            bindInt(db, update.get(), 2, markId);

            if (sqlite3_step(update.get()) != SQLITE_DONE)
            {
                throw std::runtime_error(sqlite3_errmsg(db));
            }

            if (sqlite3_changes(db) != 1)
            {
                throw std::runtime_error(
                    "Mark status changed; moderation was not saved.");
            }

            const std::string timestamp = currentTimestamp();

            Statement record(db, R"(
                INSERT INTO ModerationRecord
                    (mark_id, hod_id, decision, reason, action_at)
                VALUES (?, ?, ?, ?, ?)
            )");

            bindInt(db, record.get(), 1, markId);
            bindInt(db, record.get(), 2, hodId);
            bindText(db, record.get(), 3, decision);
            bindText(db, record.get(), 4, reason);
            bindText(db, record.get(), 5, timestamp);

            if (sqlite3_step(record.get()) != SQLITE_DONE)
            {
                throw std::runtime_error(sqlite3_errmsg(db));
            }

            std::ostringstream oldValue;
            oldValue << "status=" << oldStatus;

            std::ostringstream newValue;
            newValue << "status=" << decision
                     << "; raw=" << rawMark
                     << "; final=" << finalMark;

            Statement audit(db, R"(
                INSERT INTO AuditRecord
                    (user_id, mark_id, action, old_value,
                     new_value, reason, action_at)
                VALUES (?, ?, ?, ?, ?, ?, ?)
            )");

            bindInt(db, audit.get(), 1, hodId);
            bindInt(db, audit.get(), 2, markId);
            bindText(db, audit.get(), 3, decision);
            bindText(db, audit.get(), 4, oldValue.str());
            bindText(db, audit.get(), 5, newValue.str());
            bindText(db, audit.get(), 6, reason);
            bindText(db, audit.get(), 7, timestamp);

            if (sqlite3_step(audit.get()) != SQLITE_DONE)
            {
                throw std::runtime_error(sqlite3_errmsg(db));
            }

            execute(db, "COMMIT;");
            transactionStarted = false;
        }
        catch (...)
        {
            if (transactionStarted)
            {
                sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
            }

            throw;
        }
    }

    std::vector<MarkForModeration>
    ModerationRepository::getApprovedMarksForRelease(int hodId)
    {
        if (hodId <= 0)
            throw std::invalid_argument("Invalid HoD user ID.");

        sqlite3* db = database.getConnection();

        Statement query(db, R"(
            SELECT m.mark_id, m.submission_id, a.title, st.student_number,
                   u.full_name, m.raw_mark, m.final_mark, m.feedback, m.status
            FROM Mark m
            JOIN Submission s ON s.submission_id = m.submission_id
            JOIN Assignment a ON a.assignment_id = s.assignment_id
            JOIN Course c ON c.course_id = a.course_id
            JOIN Department d ON d.department_id = c.department_id
            JOIN HeadOfDepartment h ON h.user_id = d.hod_user_id
            JOIN Student st ON st.user_id = s.student_id
            JOIN Lecturer l ON l.user_id = m.marked_by
            JOIN "User" u ON u.user_id = l.user_id
            WHERE h.user_id = ? AND m.status = 'Approved'
            ORDER BY m.mark_id
        )");

        bindInt(db, query.get(), 1, hodId);

        std::vector<MarkForModeration> results;
        int rc;

        while ((rc = sqlite3_step(query.get())) == SQLITE_ROW)
        {
            auto txt = [&](int i)
            {
                const auto* v = sqlite3_column_text(query.get(), i);
                return v ? reinterpret_cast<const char*>(v) : "";
            };

            results.push_back({
                sqlite3_column_int(query.get(), 0),
                sqlite3_column_int(query.get(), 1),
                txt(2),
                txt(3),
                txt(4),
                sqlite3_column_double(query.get(), 5),
                sqlite3_column_double(query.get(), 6),
                txt(7),
                txt(8)
            });
        }

        if (rc != SQLITE_DONE)
            throw std::runtime_error(sqlite3_errmsg(db));

        return results;
    }

    int ModerationRepository::releaseApprovedMarks(int hodId)
    {
        if (hodId <= 0)
            throw std::invalid_argument("Invalid HoD user ID.");

        sqlite3* db = database.getConnection();
        bool started = false;

        try
        {
            execute(db, "BEGIN IMMEDIATE;");
            started = true;

            Statement check(
                db,
                "SELECT 1 FROM HeadOfDepartment WHERE user_id = ?");

            bindInt(db, check.get(), 1, hodId);

            if (sqlite3_step(check.get()) != SQLITE_ROW)
                throw std::runtime_error(
                    "This user is not registered as a Head of Department.");

            Statement marks(db, R"(
                SELECT m.mark_id, m.raw_mark, m.final_mark
                FROM Mark m
                JOIN Submission s ON s.submission_id = m.submission_id
                JOIN Assignment a ON a.assignment_id = s.assignment_id
                JOIN Course c ON c.course_id = a.course_id
                JOIN Department d ON d.department_id = c.department_id
                WHERE d.hod_user_id = ? AND m.status = 'Approved'
            )");

            bindInt(db, marks.get(), 1, hodId);

            struct Item
            {
                int id;
                double raw;
                double final;
            };

            std::vector<Item> items;
            int rc;

            while ((rc = sqlite3_step(marks.get())) == SQLITE_ROW)
            {
                items.push_back({
                    sqlite3_column_int(marks.get(), 0),
                    sqlite3_column_double(marks.get(), 1),
                    sqlite3_column_double(marks.get(), 2)
                });
            }

            if (rc != SQLITE_DONE)
                throw std::runtime_error(sqlite3_errmsg(db));

            if (items.empty())
            {
                execute(db, "COMMIT;");
                started = false;
                return 0;
            }

            const std::string timestamp = currentTimestamp();

            for (const auto& item : items)
            {
                Statement update(db, R"(
                    UPDATE Mark
                    SET status = 'Released'
                    WHERE mark_id = ? AND status = 'Approved'
                )");

                bindInt(db, update.get(), 1, item.id);

                if (sqlite3_step(update.get()) != SQLITE_DONE ||
                    sqlite3_changes(db) != 1)
                {
                    throw std::runtime_error("A mark could not be released.");
                }

                std::ostringstream oldValue;
                std::ostringstream newValue;

                oldValue << "status=Approved; raw=" << item.raw
                         << "; final=" << item.final;

                newValue << "status=Released; raw=" << item.raw
                         << "; final=" << item.final;

                Statement audit(db, R"(
                    INSERT INTO AuditRecord
                        (user_id, mark_id, action, old_value,
                         new_value, reason, action_at)
                    VALUES (?, ?, 'MarkReleased', ?, ?,
                            'HoD released approved mark', ?)
                )");

                bindInt(db, audit.get(), 1, hodId);
                bindInt(db, audit.get(), 2, item.id);
                bindText(db, audit.get(), 3, oldValue.str());
                bindText(db, audit.get(), 4, newValue.str());
                bindText(db, audit.get(), 5, timestamp);

                if (sqlite3_step(audit.get()) != SQLITE_DONE)
                    throw std::runtime_error(sqlite3_errmsg(db));
            }

            execute(db, "COMMIT;");
            started = false;

            return static_cast<int>(items.size());
        }
        catch (...)
        {
            if (started)
                sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);

            throw;
        }
    }

    void ModerationRepository::adjustMark(
        int hodId,
        int markId,
        double newRawMark,
        const std::string& reason)
    {
        if (hodId <= 0 || markId <= 0 || reason.empty() ||
            !std::isfinite(newRawMark) || newRawMark < 0)
        {
            throw std::invalid_argument(
                "A valid mark and moderation reason are required.");
        }

        sqlite3* db = database.getConnection();

        Statement details(db, R"(
            SELECT m.raw_mark, m.final_mark,
                   s.assignment_id, s.student_id, s.submitted_at,
                   a.max_mark
            FROM Mark m
            JOIN Submission s ON s.submission_id = m.submission_id
            JOIN Assignment a ON a.assignment_id = s.assignment_id
            JOIN Course c ON c.course_id = a.course_id
            JOIN Department d ON d.department_id = c.department_id
            WHERE m.mark_id = ?
              AND m.status = 'AwaitingModeration'
              AND d.hod_user_id = ?
        )");

        bindInt(db, details.get(), 1, markId);
        bindInt(db, details.get(), 2, hodId);

        if (sqlite3_step(details.get()) != SQLITE_ROW)
        {
            throw std::runtime_error(
                "Mark is not awaiting moderation in your department.");
        }

        const double oldRaw = sqlite3_column_double(details.get(), 0);
        const double oldFinal = sqlite3_column_double(details.get(), 1);
        const int assignmentId = sqlite3_column_int(details.get(), 2);
        const int studentId = sqlite3_column_int(details.get(), 3);

        const auto* submittedText =
            sqlite3_column_text(details.get(), 4);

        const std::string submitted = submittedText
            ? reinterpret_cast<const char*>(submittedText)
            : "";

        const double maxMark = sqlite3_column_double(details.get(), 5);

        if (newRawMark > maxMark)
        {
            throw std::invalid_argument(
                "Adjusted mark cannot exceed maximum mark.");
        }

        DeadlineExtensionRepository extensions(database);

        const std::string deadline =
            extensions.getEffectiveDeadline(assignmentId, studentId);

        const auto result = DeadlineCalculator::calculate(
            deadline, submitted, newRawMark, maxMark);

        // Record both raw marks and final marks in the audit trail.
        std::ostringstream oldAudit;
        std::ostringstream newAudit;

        oldAudit << "raw_mark=" << oldRaw
                 << "; final_mark=" << oldFinal;

        newAudit << "raw_mark=" << newRawMark
                 << "; final_mark=" << result.finalMark;

        const std::string oldValue = oldAudit.str();
        const std::string newValue = newAudit.str();

        execute(db, "BEGIN IMMEDIATE;");

        try
        {
            Statement update(db, R"(
                UPDATE Mark
                SET raw_mark = ?, final_mark = ?
                WHERE mark_id = ?
                  AND status = 'AwaitingModeration'
            )");

            sqlite3_bind_double(update.get(), 1, newRawMark);
            sqlite3_bind_double(update.get(), 2, result.finalMark);
            bindInt(db, update.get(), 3, markId);

            if (sqlite3_step(update.get()) != SQLITE_DONE ||
                sqlite3_changes(db) != 1)
            {
                throw std::runtime_error(
                    "Could not save moderation adjustment.");
            }

            Statement audit(db, R"(
                INSERT INTO AuditRecord
                    (user_id, mark_id, action, old_value,
                     new_value, reason, action_at)
                VALUES (?, ?, 'HoD mark adjustment', ?, ?, ?, ?)
            )");

            bindInt(db, audit.get(), 1, hodId);
            bindInt(db, audit.get(), 2, markId);
            bindText(db, audit.get(), 3, oldValue);
            bindText(db, audit.get(), 4, newValue);
            bindText(db, audit.get(), 5, reason);
            bindText(db, audit.get(), 6, currentTimestamp());

            if (sqlite3_step(audit.get()) != SQLITE_DONE)
            {
                throw std::runtime_error(
                    "Could not write audit record.");
            }

            execute(db, "COMMIT;");
        }
        catch (...)
        {
            try
            {
                execute(db, "ROLLBACK;");
            }
            catch (...)
            {
            }

            throw;
        }
    }
}
