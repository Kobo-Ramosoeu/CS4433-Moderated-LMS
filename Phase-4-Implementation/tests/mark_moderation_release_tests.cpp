#include "lms/Database.h"
#include "lms/DeadlineCalculator.h"
#include "lms/MarkingRepository.h"
#include "lms/ModerationRepository.h"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <limits>
#include <stdexcept>

int main() {
    const char* dbPath = "mark_moderation_test.db";
    std::remove(dbPath);

    try {
        {
            lms::Database db(dbPath);
            db.executeFile("database/schema.sql");

            db.execute(
                "INSERT INTO \"User\"(username,password_hash,full_name,role) VALUES"
                "('student','x','Student','Student'),"
                "('lecturer','x','Lecturer','Lecturer'),"
                "('hod','x','HoD','HoD');"
            );

            db.execute(
                "INSERT INTO Student(user_id,student_number) VALUES(1,'S001');"
            );

            db.execute(
                "INSERT INTO Lecturer(user_id,staff_number) VALUES(2,'L001');"
            );

            db.execute(
                "INSERT INTO HeadOfDepartment(user_id) VALUES(3);"
            );

            db.execute(
                "INSERT INTO Department(name,hod_user_id) VALUES('Computing',3);"
            );

            db.execute(
                "INSERT INTO Course(department_id,course_code,course_name) "
                "VALUES(1,'CS4433','Moderated LMS');"
            );

            db.execute(
                "INSERT INTO CourseLecturer(course_id,lecturer_id) VALUES(1,2);"
            );

            db.execute(
                "INSERT INTO Enrolment(student_id,course_id) VALUES(1,1);"
            );

            db.execute(
                "INSERT INTO Assignment(course_id,title,description,deadline,max_mark) "
                "VALUES(1,'Test','', '2099-11-07 23:59:00',100);"
            );

            db.execute(
                "INSERT INTO Submission(assignment_id,student_id,attempt_number,file_path,submitted_at) "
                "VALUES(1,1,1,'submission.txt','2026-10-31 12:00:00');"
            );

            lms::MarkingRepository marking(db);
            lms::ModerationRepository moderation(db);

            // Test 1: A student initially has no released marks.
            assert(marking.getReleasedMarksForStudent(1).empty());

            // Test 2: Invalid HoD ID is rejected by the moderation queue.
            bool invalidHodRejected = false;

            try {
                moderation.getMarksAwaitingModeration(0);
            } catch (const std::invalid_argument&) {
                invalidHodRejected = true;
            }

            assert(invalidHodRejected);

            // Test 3: An invalid moderation decision is rejected.
            bool invalidDecisionRejected = false;

            try {
                moderation.moderateMark(
                    3, 1, "Rejected", "Invalid decision"
                );
            } catch (const std::invalid_argument&) {
                invalidDecisionRejected = true;
            }

            assert(invalidDecisionRejected);

            // Test 4: Returning a mark requires a reason.
            bool missingReasonRejected = false;

            try {
                moderation.moderateMark(
                    3, 1, "CorrectionRequired", ""
                );
            } catch (const std::invalid_argument&) {
                missingReasonRejected = true;
            }

            assert(missingReasonRejected);

            // Test 5: An unregistered HoD cannot moderate marks.
            bool unregisteredHodRejected = false;

            try {
                moderation.moderateMark(
                    999, 1, "Approved", "Test"
                );
            } catch (const std::runtime_error&) {
                unregisteredHodRejected = true;
            }

            assert(unregisteredHodRejected);

            // Test 6: A nonexistent mark cannot be moderated.
            bool missingMarkRejected = false;

            try {
                moderation.moderateMark(
                    3, 999, "Approved", "Test"
                );
            } catch (const std::runtime_error&) {
                missingMarkRejected = true;
            }

            assert(missingMarkRejected);

            // Create a second department and HoD.
            db.execute(
                "INSERT INTO \"User\"(username,password_hash,full_name,role) "
                "VALUES('other_hod','x','Other HoD','HoD');"
            );

            db.execute(
                "INSERT INTO HeadOfDepartment(user_id) VALUES(4);"
            );

            db.execute(
                "INSERT INTO Department(name,hod_user_id) "
                "VALUES('Mathematics',4);"
            );

            // Test 7: An HoD cannot moderate another department's mark.
            bool wrongDepartmentRejected = false;

            try {
                moderation.moderateMark(
                    4, 1, "Approved", "Test"
                );
            } catch (const std::runtime_error&) {
                wrongDepartmentRejected = true;
            }

            assert(wrongDepartmentRejected);

            // Test 8: An invalid mark ID is rejected for adjustments.
            bool invalidAdjustmentRejected = false;

            try {
                moderation.adjustMark(
                    3, 0, 80.0, "Test adjustment"
                );
            } catch (const std::invalid_argument&) {
                invalidAdjustmentRejected = true;
            }

            assert(invalidAdjustmentRejected);

            // Test 9: Negative adjusted marks are rejected.
            bool negativeAdjustmentRejected = false;

            try {
                moderation.adjustMark(
                    3, 1, -5.0, "Negative mark test"
                );
            } catch (const std::invalid_argument&) {
                negativeAdjustmentRejected = true;
            }

            assert(negativeAdjustmentRejected);

            // Test 10: Infinite adjusted marks are rejected.
            bool infiniteAdjustmentRejected = false;

            try {
                moderation.adjustMark(
                    3, 1, std::numeric_limits<double>::infinity(),
                    "Infinite mark test"
                );
            } catch (const std::invalid_argument&) {
                infiniteAdjustmentRejected = true;
            }

            assert(infiniteAdjustmentRejected);

            // Test 11: NaN adjusted marks are rejected.
            bool nanAdjustmentRejected = false;

            try {
                moderation.adjustMark(
                    3, 1, std::numeric_limits<double>::quiet_NaN(),
                    "NaN mark test"
                );
            } catch (const std::invalid_argument&) {
                nanAdjustmentRejected = true;
            }

            assert(nanAdjustmentRejected);

            // Test 12: An adjustment requires a reason.
            bool emptyAdjustmentReasonRejected = false;

            try {
                moderation.adjustMark(
                    3, 1, 80.0, ""
                );
            } catch (const std::invalid_argument&) {
                emptyAdjustmentReasonRejected = true;
            }

            assert(emptyAdjustmentReasonRejected);

            // Test 13: Enter the original mark.
            const auto result = marking.enterMark(
                2, 1, 85.0, "Good work"
            );

            assert(result.finalMark == 85.0);

            // Test 14: The mark appears in the correct HoD's queue.
            auto awaiting = moderation.getMarksAwaitingModeration(3);

            assert(awaiting.size() == 1);
            assert(awaiting[0].markId == 1);
            assert(awaiting[0].assignmentTitle == "Test");
            assert(awaiting[0].studentNumber == "S001");
            assert(awaiting[0].lecturerName == "Lecturer");
            assert(awaiting[0].rawMark == 85.0);
            assert(awaiting[0].finalMark == 85.0);
            assert(awaiting[0].status == "AwaitingModeration");

            // The other department must not see this mark.
            assert(
                moderation.getMarksAwaitingModeration(4).empty()
            );

            // Test 15: An adjustment above the assignment maximum is rejected.
            bool excessiveAdjustmentRejected = false;

            try {
                moderation.adjustMark(
                    3, 1, 105.0, "Exceeds maximum"
                );
            } catch (const std::invalid_argument&) {
                excessiveAdjustmentRejected = true;
            }

            assert(excessiveAdjustmentRejected);

            // Test 16: A valid HoD adjustment updates the mark.
            moderation.adjustMark(
                3, 1, 90.0, "Corrected marking calculation"
            );

            awaiting = moderation.getMarksAwaitingModeration(3);

            assert(awaiting.size() == 1);
            assert(awaiting[0].rawMark == 90.0);
            assert(awaiting[0].finalMark == 90.0);

            // Test 17: An HoD from another department cannot adjust the mark.
            bool wrongDepartmentAdjustmentRejected = false;

            try {
                moderation.adjustMark(
                    4, 1, 95.0, "Wrong department"
                );
            } catch (const std::runtime_error&) {
                wrongDepartmentAdjustmentRejected = true;
            }

            assert(wrongDepartmentAdjustmentRejected);

            // Test 18: A mark can be returned for correction.
            moderation.moderateMark(
                3, 1, "CorrectionRequired",
                "Please review calculation"
            );

            assert(
                moderation.getMarksAwaitingModeration(3).empty()
            );

            auto returned = marking.getReturnedMarksForLecturer(2);

            assert(returned.size() == 1);

            // Test 19: A mark that is no longer awaiting moderation
            // cannot be adjusted.
            bool reviewedMarkAdjustmentRejected = false;

            try {
                moderation.adjustMark(
                    3, 1, 95.0, "Attempt after review"
                );
            } catch (const std::runtime_error&) {
                reviewedMarkAdjustmentRejected = true;
            }

            assert(reviewedMarkAdjustmentRejected);

            // Test 20: The lecturer corrects the returned mark.
            const auto corrected = marking.correctReturnedMark(
                2, 1, 90.0, "Calculation corrected"
            );

            assert(corrected.finalMark == 90.0);

            // Test 21: Approve the corrected mark.
            moderation.moderateMark(
                3, 1, "Approved", "Correction accepted"
            );

            // Approved marks are available for release.
            auto approved = moderation.getApprovedMarksForRelease(3);

            assert(approved.size() == 1);
            assert(approved[0].markId == 1);
            assert(approved[0].rawMark == 90.0);
            assert(approved[0].finalMark == 90.0);
            assert(approved[0].status == "Approved");

            // Approved marks are not yet visible to the student.
            assert(marking.getReleasedMarksForStudent(1).empty());

            // Test 22: An invalid HoD ID cannot retrieve approved marks.
            bool invalidApprovedHodRejected = false;

            try {
                moderation.getApprovedMarksForRelease(0);
            } catch (const std::invalid_argument&) {
                invalidApprovedHodRejected = true;
            }

            assert(invalidApprovedHodRejected);

            // Test 23: A mark cannot be moderated twice.
            bool duplicateModerationRejected = false;

            try {
                moderation.moderateMark(
                    3, 1, "Approved", "Duplicate approval"
                );
            } catch (const std::runtime_error&) {
                duplicateModerationRejected = true;
            }

            assert(duplicateModerationRejected);

            // Test 24: Release approved marks.
            const int released = moderation.releaseApprovedMarks(3);

            assert(released == 1);

            auto visible = marking.getReleasedMarksForStudent(1);

            assert(visible.size() == 1);
            assert(visible[0].rawMark == 90.0);
            assert(visible[0].finalMark == 90.0);

            // Test 25: Released marks no longer appear in the
            // approved-for-release queue.
            assert(
                moderation.getApprovedMarksForRelease(3).empty()
            );

            // Test 26: Releasing again when there are no approved marks
            // returns zero.
            assert(moderation.releaseApprovedMarks(3) == 0);

            // Test 27: Invalid HoD IDs cannot release marks.
            bool invalidReleaseHodRejected = false;

            try {
                moderation.releaseApprovedMarks(0);
            } catch (const std::invalid_argument&) {
                invalidReleaseHodRejected = true;
            }

            assert(invalidReleaseHodRejected);

            // Test 28: Audit history records the marking,
            // adjustment, moderation, correction and release workflow.
            sqlite3_stmt* statement = nullptr;

            const int prepareResult = sqlite3_prepare_v2(
                db.getConnection(),
                "SELECT COUNT(*) FROM AuditRecord WHERE mark_id = 1",
                -1,
                &statement,
                nullptr
            );

            assert(prepareResult == SQLITE_OK);
            assert(statement != nullptr);

            const int stepResult = sqlite3_step(statement);

            assert(stepResult == SQLITE_ROW);

            const int auditCount = sqlite3_column_int(statement, 0);

            sqlite3_finalize(statement);

            assert(auditCount >= 5);
        }

        std::remove(dbPath);

        std::cout
            << "Marking/moderation/release workflow tests passed.\n";

        return 0;
    }
    catch (const std::exception& error) {
        std::remove(dbPath);
        std::cerr << error.what() << '\n';
        return 1;
    }
}