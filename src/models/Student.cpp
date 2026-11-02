#include "lms/models/Student.h"

namespace lms
{
    Student::Student(
        int userId,
        const std::string& username,
        const std::string& passwordHash,
        const std::string& fullName
    )
        : User(userId, username, passwordHash, fullName)
    {
    }

    std::string Student::getRole() const
    {
        return "Student";
    }
}
