#include "lms/AdministrationRepository.h"
#include "lms/DeadlineExtensionRepository.h"
#include <cassert>
#include <cstdio>
#include <iostream>
#include <stdexcept>

int main() {
    const char* dbPath = "lms_repository_test.db";
    std::remove(dbPath);
    try {
      {
        lms::Database db(dbPath);
        db.executeFile("database/schema.sql");
        lms::AdministrationRepository admin(db);

        db.execute("INSERT INTO \"User\"(username,password_hash,full_name,role,is_active) VALUES"
                   "('root','hash','Root Admin','Administrator',1);");
        const int adminId = static_cast<int>(sqlite3_last_insert_rowid(db.getConnection()));
        admin.createAccount("root2", "hash", "Second Admin", "Administrator", "");
        admin.createAccount("lecturer1", "hash", "Lecturer One", "Lecturer", "LEC001");
        admin.createAccount("student1", "hash", "Student One", "Student", "STU001");
        admin.createAccount("hod1", "hash", "HoD One", "HoD", "HOD001");

        int lecturerId = 0, studentId = 0, hodId = 0;
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db.getConnection(), "SELECT user_id,role FROM \"User\" ORDER BY user_id;", -1, &stmt, nullptr);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const int id = sqlite3_column_int(stmt, 0);
            const char* role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            if (std::string(role) == "Lecturer") lecturerId = id;
            if (std::string(role) == "Student") studentId = id;
            if (std::string(role) == "HoD") hodId = id;
        }
        sqlite3_finalize(stmt);

        db.execute("INSERT INTO Department(name) VALUES('Computing');");
        db.execute("INSERT INTO Course(department_id,course_code,course_name) VALUES(1,'CS4433','Moderated LMS');");
        db.execute("INSERT INTO Assignment(course_id,title,description,deadline,max_mark) VALUES(1,'Test Assignment','Integration test','2026-11-07 23:59:00',100);");
        admin.assignLecturer(1, lecturerId);
        admin.enrolStudent(1, studentId);

        lms::DeadlineExtensionRepository extensions(db);
        extensions.grantExtension(lecturerId, 1, 0, "2026-11-09 12:00:00", "Whole class extension");
        assert(extensions.getEffectiveDeadline(1, studentId) == "2026-11-09 12:00:00");
        extensions.grantExtension(lecturerId, 1, studentId, "2026-11-10 12:00:00", "Individual extension");
        assert(extensions.getEffectiveDeadline(1, studentId) == "2026-11-10 12:00:00");
        assert(extensions.getEffectiveDeadline(1, 9999) == "2026-11-09 12:00:00");

        admin.setUserActive(adminId, false);
        int secondAdminId = 0;
        sqlite3_prepare_v2(db.getConnection(), "SELECT user_id FROM \"User\" WHERE username=\'root2\';", -1, &stmt, nullptr);
        if (sqlite3_step(stmt) == SQLITE_ROW) secondAdminId = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        bool lastAdminBlocked = false;
        try { admin.setUserActive(secondAdminId, false); }
        catch (const std::runtime_error&) { lastAdminBlocked = true; }
        assert(lastAdminBlocked);

        admin.setUserActive(hodId, false);
        bool inactiveHodBlocked = false;
        try { admin.setHeadOfDepartment(1, hodId); }
        catch (const std::runtime_error&) { inactiveHodBlocked = true; }
        assert(inactiveHodBlocked);

      } // Close SQLite connection before removing the test database (Windows compatibility).
        std::remove(dbPath);
        std::cout << "Repository integration tests passed.\n";
        return 0;
    } catch (const std::exception& e) {
        std::remove(dbPath);
        std::cerr << "Repository integration test failed: " << e.what() << '\n';
        return 1;
    }
}
