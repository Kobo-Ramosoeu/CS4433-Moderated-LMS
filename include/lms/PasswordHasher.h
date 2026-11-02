#ifndef LMS_PASSWORD_HASHER_H
#define LMS_PASSWORD_HASHER_H

#include <string>

namespace lms
{
    class PasswordHasher
    {
    public:
        static std::string hashPassword(
            const std::string& password
        );

        static bool verifyPassword(
            const std::string& password,
            const std::string& storedHash
        );
    };
}

#endif
