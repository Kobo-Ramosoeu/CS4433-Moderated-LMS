#ifndef LMS_ADMINISTRATION_REPOSITORY_H
#define LMS_ADMINISTRATION_REPOSITORY_H

#include "lms/Database.h"
#include <string>
#include <vector>

namespace lms {

struct AdminUserRow {
    int id{};
    std::string username, fullName, role;
    int active{};
};

struct DepartmentRow {
    int id{};
    std::string name;
    int hodId{};
};

struct CourseRow {
    int id{}, departmentId{};
    std::string code, name, department;
};

struct StudentCourseRow {
    std::string code, name, department;
};

struct ReleasedMarkRow {
    std::string courseCode, assignment, feedback, markedAt;
    double rawMark{}, finalMark{}, maxMark{};
};

class AdministrationRepository {
    Database& database;

public:
    explicit AdministrationRepository(Database& db);

    void createAccount(
        const std::string& username,
        const std::string& hash,
        const std::string& fullName,
        const std::string& role,
        const std::string& identifier
    );

    std::vector<AdminUserRow> getUsers();
    void setUserActive(int userId, bool active);

    void updateAccount(
        int userId,
        const std::string& username,
        const std::string& fullName
    );

    void createDepartment(const std::string& name);
    void updateDepartment(int departmentId, const std::string& name);
    std::vector<DepartmentRow> getDepartments();

    void setHeadOfDepartment(int departmentId, int hodUserId);

    void createCourse(
        int departmentId,
        const std::string& code,
        const std::string& name
    );

    void updateCourse(
        int courseId,
        int departmentId,
        const std::string& code,
        const std::string& name
    );

    std::vector<CourseRow> getCourses();

    void assignLecturer(int courseId, int lecturerId);
    void enrolStudent(int courseId, int studentId);

    std::vector<StudentCourseRow> getStudentCourses(int studentId);
    std::vector<ReleasedMarkRow> getReleasedMarks(int studentId);
};

}

#endif