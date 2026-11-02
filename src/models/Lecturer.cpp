#include "lms/models/Lecturer.h"

namespace lms
{
    Lecturer::Lecturer(
        int userId,
        const std::string& username,
        const std::string& passwordHash,
        const std::string& fullName
    )
        : User(userId, username, passwordHash, fullName)
    {
    }

    std::string Lecturer::getRole() const
    {
        return "Lecturer";
    }
}
