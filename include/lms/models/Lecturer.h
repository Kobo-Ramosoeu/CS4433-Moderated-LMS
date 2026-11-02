#ifndef LMS_LECTURER_H
#define LMS_LECTURER_H

#include "lms/User.h"

namespace lms
{
    class Lecturer : public User
    {
    public:
        Lecturer(
            int userId,
            const std::string& username,
            const std::string& passwordHash,
            const std::string& fullName
        );

        std::string getRole() const override;
    };
}

#endif
