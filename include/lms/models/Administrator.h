#ifndef LMS_ADMINISTRATOR_H
#define LMS_ADMINISTRATOR_H

#include "lms/User.h"

namespace lms
{
    class Administrator : public User
    {
    public:
        Administrator(
            int userId,
            const std::string& username,
            const std::string& passwordHash,
            const std::string& fullName
        );

        std::string getRole() const override;
    };
}

#endif
