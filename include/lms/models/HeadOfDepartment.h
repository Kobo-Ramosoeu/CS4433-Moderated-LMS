#ifndef LMS_HEAD_OF_DEPARTMENT_H
#define LMS_HEAD_OF_DEPARTMENT_H

#include "lms/User.h"

namespace lms
{
    class HeadOfDepartment : public User
    {
    public:
        HeadOfDepartment(
            int userId,
            const std::string& username,
            const std::string& passwordHash,
            const std::string& fullName
        );

        std::string getRole() const override;
    };
}

#endif
