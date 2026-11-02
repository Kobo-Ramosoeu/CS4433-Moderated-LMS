#include "lms/models/HeadOfDepartment.h"

namespace lms
{
    HeadOfDepartment::HeadOfDepartment(
        int userId,
        const std::string& username,
        const std::string& passwordHash,
        const std::string& fullName
    )
        : User(userId, username, passwordHash, fullName)
    {
    }

    std::string HeadOfDepartment::getRole() const
    {
        return "HoD";
    }
}
