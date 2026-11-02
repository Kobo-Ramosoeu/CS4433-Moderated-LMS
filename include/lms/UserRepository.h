
#ifndef LMS_USER_REPOSITORY_H
#define LMS_USER_REPOSITORY_H

#include "lms/Database.h"

#include <string>

namespace lms
{
struct LoginDetails
{
    int userId{};
    std::string username;
    std::string passwordHash;
    std::string fullName;
    std::string role;
};

    class UserRepository
    {
    private:
        Database& database;

    public:
        explicit UserRepository(Database& database);

        bool usernameExists(const std::string& username);

        void createUser(
            const std::string& username,
            const std::string& passwordHash,
            const std::string& fullName,
            const std::string& role
        );

        bool findLoginDetails(
            const std::string& username,
            LoginDetails& details
        );
    };
}

#endif
