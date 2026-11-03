
#include "lms/AdministrationRepository.h"
#include "lms/DeadlineExtensionRepository.h"

#include <cassert>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>

int main()
{
    const char* dbPath = "lms_repository_test.db";
    std::remove(dbPath);

    try
    {
        {
            lms::Database db(dbPath);
            db.executeFile("database/schema.sql");

            lms::AdministrationRepository admin(db);

            // --------------------------------------------------
            // 1. Create accounts
            // --------------------------------------------------

            db.execute(
                "INSERT INTO \"User\""
                "(username,password_hash,full_name,role,is_active) "
                "VALUES('root','hash','Root Admin','Administrator',1);"
            );

            const int adminId = static_cast<int>(
                sqlite3_last_insert_rowid(db.getConnection())
            );

            admin.createAccount(
                "root2", "hash", "Second Admin", "Administrator", ""
            );

            admin.createAccount(
                "lecturer1", "hash", "Lecturer One", "Lecturer", "LEC001"
            );

            admin.createAccount(
                "student1", "hash", "Student One", "Student", "STU001"
            );

            admin.createAccount(
                "hod1", "hash", "HoD One", "HoD", "HOD001"
            );

            int secondAdminId = 0;
            int lecturerId = 0;
            int studentId = 0;
            int hodId = 0;

            sqlite3_stmt* stmt = nullptr;

            sqlite3_prepare_v2(
                db.getConnection(),
                "SELECT user_id, username FROM \"User\";",
                -1, &stmt, nullptr
            );

            while (sqlite3_step(stmt) == SQLITE_ROW)
            {
                const int id = sqlite3_column_int(stmt, 0);
                const char* username = reinterpret_cast<const char*>(
                    sqlite3_column_text(stmt, 1)
                );

                const std::string name = username ? username : "";

                if (name == "root2") secondAdminId = id;
                if (name == "lecturer1") lecturerId = id;
                if (name == "student1") studentId = id;
                if (name == "hod1") hodId = id;
            }

            sqlite3_finalize(stmt);

            assert(secondAdminId > 0);
            assert(lecturerId > 0);
            assert(studentId > 0);
            assert(hodId > 0);

            // Invalid account details must be rejected.
            bool invalidAccountRejected = false;

            try
            {
                admin.createAccount("", "hash", "Invalid", "Student", "S002");
            }
            catch (const std::invalid_argument&)
            {
                invalidAccountRejected = true;
            }

            assert(invalidAccountRejected);

            bool invalidRoleRejected = false;

            try
            {
                admin.createAccount(
                    "badrole", "hash", "Bad Role", "Unknown", ""
                );
            }
            catch (const std::invalid_argument&)
            {
                invalidRoleRejected = true;
            }

            assert(invalidRoleRejected);

            // --------------------------------------------------
            // 2. List and update accounts
            // --------------------------------------------------

            auto users = admin.getUsers();
            assert(users.size() == 5);

            admin.updateAccount(
                lecturerId, "lecturer.updated", "Updated Lecturer"
            );

            users = admin.getUsers();

            bool lecturerUpdated = false;

            for (const auto& user : users)
            {
                if (user.id == lecturerId)
                {
                    lecturerUpdated =
                        user.username == "lecturer.updated" &&
                        user.fullName == "Updated Lecturer";
                }
            }

            assert(lecturerUpdated);

            bool invalidUpdateRejected = false;

            try
            {
                admin.updateAccount(0, "name", "Full Name");
            }
            catch (const std::invalid_argument&)
            {
                invalidUpdateRejected = true;
            }

            assert(invalidUpdateRejected);

            bool missingUserRejected = false;

            try
            {
                admin.updateAccount(99999, "missing", "Missing User");
            }
            catch (const std::runtime_error&)
            {
                missingUserRejected = true;
            }

            assert(missingUserRejected);

            // --------------------------------------------------
            // 3. Department management
            // --------------------------------------------------

            admin.createDepartment("Computing");

            auto departments = admin.getDepartments();
            assert(departments.size() == 1);
            assert(departments[0].name == "Computing");

            const int departmentId = departments[0].id;

            admin.updateDepartment(departmentId, "Computer Science");

            departments = admin.getDepartments();
            assert(departments[0].name == "Computer Science");

            bool emptyDepartmentRejected = false;

            try
            {
                admin.createDepartment("");
            }
            catch (const std::invalid_argument&)
            {
                emptyDepartmentRejected = true;
            }

            assert(emptyDepartmentRejected);

            bool missingDepartmentRejected = false;

            try
            {
                admin.updateDepartment(99999, "Missing Department");
            }
            catch (const std::runtime_error&)
            {
                missingDepartmentRejected = true;
            }

            assert(missingDepartmentRejected);

            // Assign the active HoD to the department.
            admin.setHeadOfDepartment(departmentId, hodId);

            departments = admin.getDepartments();
            assert(departments[0].hodId == hodId);

            // --------------------------------------------------
            // 4. Course management
            // --------------------------------------------------

            admin.createCourse(
                departmentId, "CS4433", "Moderated LMS"
            );

            auto courses = admin.getCourses();

            assert(courses.size() == 1);
            assert(courses[0].code == "CS4433");
            assert(courses[0].name == "Moderated LMS");
            assert(courses[0].department == "Computer Science");

            const int courseId = courses[0].id;

            admin.updateCourse(
                courseId, departmentId, "CS4434", "Advanced LMS"
            );

            courses = admin.getCourses();

            assert(courses[0].code == "CS4434");
            assert(courses[0].name == "Advanced LMS");

            bool emptyCourseRejected = false;

            try
            {
                admin.createCourse(departmentId, "", "Invalid Course");
            }
            catch (const std::invalid_argument&)
            {
                emptyCourseRejected = true;
            }

            assert(emptyCourseRejected);

            bool missingCourseRejected = false;

            try
            {
                admin.updateCourse(
                    99999, departmentId, "CS9999", "Missing Course"
                );
            }
            catch (const std::runtime_error&)
            {
                missingCourseRejected = true;
            }

            assert(missingCourseRejected);

            // --------------------------------------------------
            // 5. Lecturer assignment and student enrolment
            // --------------------------------------------------

            admin.assignLecturer(courseId, lecturerId);
            admin.enrolStudent(courseId, studentId);

            auto studentCourses = admin.getStudentCourses(studentId);

            assert(studentCourses.size() == 1);
            assert(studentCourses[0].code == "CS4434");
            assert(studentCourses[0].name == "Advanced LMS");

            bool duplicateLecturerRejected = false;

            try
            {
                admin.assignLecturer(courseId, lecturerId);
            }
            catch (const std::runtime_error&)
            {
                duplicateLecturerRejected = true;
            }

            assert(duplicateLecturerRejected);

            bool duplicateEnrolmentRejected = false;

            try
            {
                admin.enrolStudent(courseId, studentId);
            }
            catch (const std::runtime_error&)
            {
                duplicateEnrolmentRejected = true;
            }

            assert(duplicateEnrolmentRejected);

            // --------------------------------------------------
            // 6. Deadline extensions
            // --------------------------------------------------

            lms::DeadlineExtensionRepository extensions(db);

            db.execute(
                "INSERT INTO Assignment"
                "(course_id,title,description,deadline,max_mark) "
                "VALUES(1,'Test Assignment','Integration test',"
                "'2026-11-07 23:59:00',100);"
            );

            extensions.grantExtension(
                lecturerId, 1, 0,
                "2026-11-09 12:00:00",
                "Whole class extension"
            );

            assert(
                extensions.getEffectiveDeadline(1, studentId) ==
                "2026-11-09 12:00:00"
            );

            extensions.grantExtension(
                lecturerId, 1, studentId,
                "2026-11-10 12:00:00",
                "Individual extension"
            );

            assert(
                extensions.getEffectiveDeadline(1, studentId) ==
                "2026-11-10 12:00:00"
            );

            assert(
                extensions.getEffectiveDeadline(1, 9999) ==
                "2026-11-09 12:00:00"
            );

            // --------------------------------------------------
            // 7. Released marks query
            // --------------------------------------------------

            // No marks have been released in this test database.
            assert(admin.getReleasedMarks(studentId).empty());

            // --------------------------------------------------
            // 8. Account activation and HoD protection
            // --------------------------------------------------

            admin.setUserActive(adminId, false);

            bool lastAdminBlocked = false;

            try
            {
                admin.setUserActive(secondAdminId, false);
            }
            catch (const std::runtime_error&)
            {
                lastAdminBlocked = true;
            }

            assert(lastAdminBlocked);

            // An inactive HoD must not be assigned.
            admin.setUserActive(hodId, false);

            bool inactiveHodBlocked = false;

            try
            {
                admin.setHeadOfDepartment(departmentId, hodId);
            }
            catch (const std::runtime_error&)
            {
                inactiveHodBlocked = true;
            }

            assert(inactiveHodBlocked);
        }

        // Close the SQLite connection before deleting the test database.
        std::remove(dbPath);

        std::cout << "Repository integration tests passed.\n";
        return 0;
    }
    catch (const std::exception& e)
    {
        std::remove(dbPath);

        std::cerr
            << "Repository integration test failed: "
            << e.what() << '\n';

        return 1;
    }
}
