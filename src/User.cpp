#include "lms/User.h"

namespace lms {

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

const std::string& User::getUsername() const
{
    return username;
}

const std::string& User::getFullName() const
{
    return fullName;
}

}