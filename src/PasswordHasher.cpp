#include "lms/PasswordHasher.h"

#include <argon2.h>
#include <windows.h>
#include <bcrypt.h>

#include <stdexcept>
#include <vector>

namespace lms
{
    std::string PasswordHasher::hashPassword(
        const std::string& password
    )
    {
        if (password.empty())
        {
            throw std::invalid_argument(
                "Password cannot be empty."
            );
        }

        constexpr unsigned int saltLength = 16;
        constexpr unsigned int hashLength = 32;
        constexpr unsigned int iterations = 3;
        constexpr unsigned int memoryCost = 65536;
        constexpr unsigned int parallelism = 1;

        unsigned char salt[saltLength];

        NTSTATUS randomResult = BCryptGenRandom(
            nullptr,
            salt,
            saltLength,
            BCRYPT_USE_SYSTEM_PREFERRED_RNG
        );

        if (randomResult < 0)
        {
            throw std::runtime_error(
                "Secure salt generation failed."
            );
        }

        std::size_t encodedLength = argon2_encodedlen(
            iterations,
            memoryCost,
            parallelism,
            saltLength,
            hashLength,
            Argon2_id
        );

        std::vector<char> encodedHash(encodedLength);

        int result = argon2id_hash_encoded(
            iterations,
            memoryCost,
            parallelism,
            password.data(),
            password.size(),
            salt,
            saltLength,
            hashLength,
            encodedHash.data(),
            encodedHash.size()
        );

        if (result != ARGON2_OK)
        {
            throw std::runtime_error(
                "Password hashing failed."
            );
        }

        return std::string(encodedHash.data());
    }

    bool PasswordHasher::verifyPassword(
        const std::string& password,
        const std::string& storedHash
    )
    {
        if (password.empty() || storedHash.empty())
        {
            return false;
        }

        int result = argon2id_verify(
            storedHash.c_str(),
            password.data(),
            password.size()
        );

        return result == ARGON2_OK;
    }
}
