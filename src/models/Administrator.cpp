#include "lms/models/Administrator.h"

namespace lms
{
    Administrator::Administrator(
        int userId,
        const std::string& username,
        const std::string& passwordHash,
        const std::string& fullName
    )
        : User(userId, username, passwordHash, fullName)
    {
    }

    std::string Administrator::getRole() const
    {
        return "Administrator";
    }
}
