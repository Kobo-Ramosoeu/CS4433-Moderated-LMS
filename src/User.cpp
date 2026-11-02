#include "lms/User.h"

namespace lms
{
    User::User(
        int userId,
        const std::string& username,
        const std::string& passwordHash,
        const std::string& fullName
    )
        : userId(userId),
          username(username),
          passwordHash(passwordHash),
          fullName(fullName)
    {
    }

    int User::getUserId() const
    {
        return userId;
    }

    std::string User::getUsername() const
    {
        return username;
    }

    std::string User::getFullName() const
    {
        return fullName;
    }
}
