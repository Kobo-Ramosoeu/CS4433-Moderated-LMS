
#include "lms/AdministrationRepository.h"

#include <sqlite3.h>
#include <memory>
#include <stdexcept>

namespace lms {

namespace {

using St = std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>;

St prep(sqlite3* db, const char* sql)
{
    sqlite3_stmt* p = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &p, nullptr) != SQLITE_OK)
        throw std::runtime_error(sqlite3_errmsg(db));

    return St(p, sqlite3_finalize);
}

void txt(sqlite3_stmt* s, int i, const std::string& v)
{
    sqlite3_bind_text(s, i, v.c_str(), -1, SQLITE_TRANSIENT);
}

std::string col(sqlite3_stmt* s, int i)
{
    auto p = sqlite3_column_text(s, i);
    return p ? reinterpret_cast<const char*>(p) : "";
}

void done(sqlite3* db, sqlite3_stmt* s, const char* message)
{
    if (sqlite3_step(s) != SQLITE_DONE)
        throw std::runtime_error(
            std::string(message) + ": " + sqlite3_errmsg(db)
        );
}

}

// Constructor

AdministrationRepository::AdministrationRepository(Database& db)
    : database(db)
{
}

// Account management

void AdministrationRepository::createAccount(
    const std::string& username,
    const std::string& hash,
    const std::string& fullName,
    const std::string& role,
    const std::string& identifier)
{
    if (username.empty() || hash.empty() || fullName.empty())
        throw std::invalid_argument("All account fields are required.");

    if (role != "Student" && role != "Lecturer" &&
        role != "HoD" && role != "Administrator")
        throw std::invalid_argument("Invalid role.");

    if ((role == "Student" || role == "Lecturer") &&
        identifier.empty())
        throw std::invalid_argument(
            "Student/staff number is required."
        );

    sqlite3* db = database.getConnection();
    database.execute("BEGIN IMMEDIATE;");

    try
    {
        auto u = prep(
            db,
            "INSERT INTO \"User\""
            "(username,password_hash,full_name,role,is_active) "
            "VALUES(?,?,?,?,1);"
        );

        txt(u.get(), 1, username);
        txt(u.get(), 2, hash);
        txt(u.get(), 3, fullName);
        txt(u.get(), 4, role);
        done(db, u.get(), "Could not create user");

        int id = static_cast<int>(sqlite3_last_insert_rowid(db));

        if (role == "Student")
        {
            auto s = prep(
                db,
                "INSERT INTO Student(user_id,student_number) VALUES(?,?);"
            );
            sqlite3_bind_int(s.get(), 1, id);
            txt(s.get(), 2, identifier);
            done(db, s.get(), "Could not create student profile");
        }
        else if (role == "Lecturer")
        {
            auto s = prep(
                db,
                "INSERT INTO Lecturer(user_id,staff_number) VALUES(?,?);"
            );
            sqlite3_bind_int(s.get(), 1, id);
            txt(s.get(), 2, identifier);
            done(db, s.get(), "Could not create lecturer profile");
        }
        else if (role == "HoD")
        {
            auto s = prep(
                db,
                "INSERT INTO HeadOfDepartment(user_id) VALUES(?);"
            );
            sqlite3_bind_int(s.get(), 1, id);
            done(db, s.get(), "Could not create HoD profile");
        }

        database.execute("COMMIT;");
    }
    catch (...)
    {
        try { database.execute("ROLLBACK;"); } catch (...) {}
        throw;
    }
}

std::vector<AdminUserRow> AdministrationRepository::getUsers()
{
    std::vector<AdminUserRow> v;

    auto s = prep(
        database.getConnection(),
        "SELECT user_id,username,full_name,role,COALESCE(is_active,1) "
        "FROM \"User\" ORDER BY user_id;"
    );

    int r;
    while ((r = sqlite3_step(s.get())) == SQLITE_ROW)
    {
        v.push_back({
            sqlite3_column_int(s.get(), 0),
            col(s.get(), 1),
            col(s.get(), 2),
            col(s.get(), 3),
            sqlite3_column_int(s.get(), 4)
        });
    }

    if (r != SQLITE_DONE)
        throw std::runtime_error(sqlite3_errmsg(database.getConnection()));

    return v;
}

void AdministrationRepository::updateAccount(
    int id,
    const std::string& username,
    const std::string& fullName)
{
    if (id <= 0 || username.empty() || fullName.empty())
        throw std::invalid_argument(
            "User ID, username and full name are required."
        );

    auto s = prep(
        database.getConnection(),
        "UPDATE \"User\" SET username=?,full_name=? WHERE user_id=?;"
    );

    txt(s.get(), 1, username);
    txt(s.get(), 2, fullName);
    sqlite3_bind_int(s.get(), 3, id);

    done(database.getConnection(), s.get(), "Could not update account");

    if (sqlite3_changes(database.getConnection()) == 0)
        throw std::runtime_error("User account not found.");
}

// Account activation and last-administrator protection

void AdministrationRepository::setUserActive(int id, bool active)
{
    sqlite3* db = database.getConnection();
    database.execute("BEGIN IMMEDIATE;");

    try
    {
        auto user = prep(
            db,
            "SELECT role,COALESCE(is_active,1) "
            "FROM \"User\" WHERE user_id=?;"
        );

        sqlite3_bind_int(user.get(), 1, id);

        if (sqlite3_step(user.get()) != SQLITE_ROW)
            throw std::runtime_error("User account not found.");

        std::string role = col(user.get(), 0);
        bool currentlyActive = sqlite3_column_int(user.get(), 1) == 1;

        if (!active && role == "Administrator" && currentlyActive)
        {
            auto count = prep(
                db,
                "SELECT COUNT(*) FROM \"User\" "
                "WHERE role='Administrator' "
                "AND COALESCE(is_active,1)=1;"
            );

            if (sqlite3_step(count.get()) != SQLITE_ROW)
                throw std::runtime_error(
                    "Could not check active administrators."
                );

            int activeAdmins = sqlite3_column_int(count.get(), 0);

            if (activeAdmins <= 1)
                throw std::runtime_error(
                    "Cannot deactivate the last active Administrator."
                );
        }

        auto update = prep(
            db,
            "UPDATE \"User\" SET is_active=? WHERE user_id=?;"
        );

        sqlite3_bind_int(update.get(), 1, active ? 1 : 0);
        sqlite3_bind_int(update.get(), 2, id);

        done(db, update.get(), "Could not update account");

        database.execute("COMMIT;");
    }
    catch (...)
    {
        try { database.execute("ROLLBACK;"); } catch (...) {}
        throw;
    }
}

// Department management

void AdministrationRepository::createDepartment(const std::string& n)
{
    if (n.empty())
        throw std::invalid_argument("Department name is required.");

    auto s = prep(
        database.getConnection(),
        "INSERT INTO Department(name) VALUES(?);"
    );

    txt(s.get(), 1, n);
    done(database.getConnection(), s.get(), "Could not create department");
}

void AdministrationRepository::updateDepartment(
    int id,
    const std::string& name)
{
    if (id <= 0 || name.empty())
        throw std::invalid_argument(
            "Department ID and name are required."
        );

    auto s = prep(
        database.getConnection(),
        "UPDATE Department SET name=? WHERE department_id=?;"
    );

    txt(s.get(), 1, name);
    sqlite3_bind_int(s.get(), 2, id);

    done(database.getConnection(), s.get(), "Could not update department");

    if (sqlite3_changes(database.getConnection()) == 0)
        throw std::runtime_error("Department not found.");
}

std::vector<DepartmentRow> AdministrationRepository::getDepartments()
{
    std::vector<DepartmentRow> v;

    auto s = prep(
        database.getConnection(),
        "SELECT department_id,name,COALESCE(hod_user_id,0) "
        "FROM Department ORDER BY department_id;"
    );

    int r;
    while ((r = sqlite3_step(s.get())) == SQLITE_ROW)
    {
        v.push_back({
            sqlite3_column_int(s.get(), 0),
            col(s.get(), 1),
            sqlite3_column_int(s.get(), 2)
        });
    }

    if (r != SQLITE_DONE)
        throw std::runtime_error(sqlite3_errmsg(database.getConnection()));

    return v;
}

// HoD assignment: only active HoD accounts may be assigned

void AdministrationRepository::setHeadOfDepartment(int did, int uid)
{
    auto s = prep(
        database.getConnection(),
        "UPDATE Department SET hod_user_id=? "
        "WHERE department_id=? "
        "AND EXISTS("
        "SELECT 1 FROM HeadOfDepartment h "
        "JOIN \"User\" u ON u.user_id=h.user_id "
        "WHERE h.user_id=? "
        "AND u.role='HoD' "
        "AND COALESCE(u.is_active,1)=1"
        ");"
    );

    sqlite3_bind_int(s.get(), 1, uid);
    sqlite3_bind_int(s.get(), 2, did);
    sqlite3_bind_int(s.get(), 3, uid);

    done(database.getConnection(), s.get(), "Could not assign HoD");

    if (sqlite3_changes(database.getConnection()) == 0)
        throw std::runtime_error(
            "Department or active HoD account not found."
        );
}

// Course management

void AdministrationRepository::createCourse(
    int did,
    const std::string& code,
    const std::string& name)
{
    if (code.empty() || name.empty())
        throw std::invalid_argument(
            "Course code and name are required."
        );

    auto s = prep(
        database.getConnection(),
        "INSERT INTO Course(department_id,course_code,course_name) "
        "VALUES(?,?,?);"
    );

    sqlite3_bind_int(s.get(), 1, did);
    txt(s.get(), 2, code);
    txt(s.get(), 3, name);

    done(database.getConnection(), s.get(), "Could not create course");
}

void AdministrationRepository::updateCourse(
    int id,
    int did,
    const std::string& code,
    const std::string& name)
{
    if (id <= 0 || did <= 0 || code.empty() || name.empty())
        throw std::invalid_argument("All course details are required.");

    auto s = prep(
        database.getConnection(),
        "UPDATE Course SET department_id=?,course_code=?,"
        "course_name=? WHERE course_id=?;"
    );

    sqlite3_bind_int(s.get(), 1, did);
    txt(s.get(), 2, code);
    txt(s.get(), 3, name);
    sqlite3_bind_int(s.get(), 4, id);

    done(database.getConnection(), s.get(), "Could not update course");

    if (sqlite3_changes(database.getConnection()) == 0)
        throw std::runtime_error("Course not found.");
}

std::vector<CourseRow> AdministrationRepository::getCourses()
{
    std::vector<CourseRow> v;

    auto s = prep(
        database.getConnection(),
        "SELECT c.course_id,c.department_id,c.course_code,"
        "c.course_name,d.name "
        "FROM Course c JOIN Department d "
        "ON d.department_id=c.department_id "
        "ORDER BY c.course_id;"
    );

    int r;
    while ((r = sqlite3_step(s.get())) == SQLITE_ROW)
    {
        v.push_back({
            sqlite3_column_int(s.get(), 0),
            sqlite3_column_int(s.get(), 1),
            col(s.get(), 2),
            col(s.get(), 3),
            col(s.get(), 4)
        });
    }

    if (r != SQLITE_DONE)
        throw std::runtime_error(sqlite3_errmsg(database.getConnection()));

    return v;
}

void AdministrationRepository::assignLecturer(int cid, int lid)
{
    auto s = prep(
        database.getConnection(),
        "INSERT OR IGNORE INTO CourseLecturer(course_id,lecturer_id) "
        "SELECT ?,? WHERE "
        "EXISTS(SELECT 1 FROM Course WHERE course_id=?) "
        "AND EXISTS(SELECT 1 FROM Lecturer WHERE user_id=?);"
    );

    sqlite3_bind_int(s.get(), 1, cid);
    sqlite3_bind_int(s.get(), 2, lid);
    sqlite3_bind_int(s.get(), 3, cid);
    sqlite3_bind_int(s.get(), 4, lid);

    done(database.getConnection(), s.get(), "Could not assign lecturer");

    if (sqlite3_changes(database.getConnection()) == 0)
        throw std::runtime_error(
            "Course or lecturer not found (or assignment already exists)."
        );
}

void AdministrationRepository::enrolStudent(int cid, int sid)
{
    auto s = prep(
        database.getConnection(),
        "INSERT OR IGNORE INTO Enrolment(student_id,course_id) "
        "SELECT ?,? WHERE "
        "EXISTS(SELECT 1 FROM Student WHERE user_id=?) "
        "AND EXISTS(SELECT 1 FROM Course WHERE course_id=?);"
    );

    sqlite3_bind_int(s.get(), 1, sid);
    sqlite3_bind_int(s.get(), 2, cid);
    sqlite3_bind_int(s.get(), 3, sid);
    sqlite3_bind_int(s.get(), 4, cid);

    done(database.getConnection(), s.get(), "Could not enrol student");

    if (sqlite3_changes(database.getConnection()) == 0)
        throw std::runtime_error(
            "Student or course not found (or student already enrolled)."
        );
}

// Student course information

std::vector<StudentCourseRow>
AdministrationRepository::getStudentCourses(int sid)
{
    std::vector<StudentCourseRow> v;

    auto s = prep(
        database.getConnection(),
        "SELECT c.course_code,c.course_name,d.name "
        "FROM Enrolment e "
        "JOIN Course c ON c.course_id=e.course_id "
        "JOIN Department d ON d.department_id=c.department_id "
        "WHERE e.student_id=? ORDER BY c.course_code;"
    );

    sqlite3_bind_int(s.get(), 1, sid);

    int r;
    while ((r = sqlite3_step(s.get())) == SQLITE_ROW)
    {
        v.push_back({
            col(s.get(), 0),
            col(s.get(), 1),
            col(s.get(), 2)
        });
    }

    if (r != SQLITE_DONE)
        throw std::runtime_error(sqlite3_errmsg(database.getConnection()));

    return v;
}

// Released marks

std::vector<ReleasedMarkRow>
AdministrationRepository::getReleasedMarks(int sid)
{
    std::vector<ReleasedMarkRow> v;

    auto s = prep(
        database.getConnection(),
        "SELECT c.course_code,a.title,m.feedback,m.marked_at,"
        "m.raw_mark,m.final_mark,a.max_mark "
        "FROM Mark m "
        "JOIN Submission sub ON sub.submission_id=m.submission_id "
        "JOIN Assignment a ON a.assignment_id=sub.assignment_id "
        "JOIN Course c ON c.course_id=a.course_id "
        "JOIN Enrolment e ON e.student_id=sub.student_id "
        "AND e.course_id=a.course_id "
        "WHERE sub.student_id=? AND m.status='Released' "
        "ORDER BY m.marked_at DESC;"
    );

    sqlite3_bind_int(s.get(), 1, sid);

    int r;
    while ((r = sqlite3_step(s.get())) == SQLITE_ROW)
    {
        v.push_back({
            col(s.get(), 0),
            col(s.get(), 1),
            col(s.get(), 2),
            col(s.get(), 3),
            sqlite3_column_double(s.get(), 4),
            sqlite3_column_double(s.get(), 5),
            sqlite3_column_double(s.get(), 6)
        });
    }

    if (r != SQLITE_DONE)
        throw std::runtime_error(sqlite3_errmsg(database.getConnection()));

    return v;
}

}
