#ifndef LMS_STUDENT_H
#define LMS_STUDENT_H

#include "lms/User.h"

namespace lms
{
    class Student : public User
    {
    public:
        Student(
            int userId,
            const std::string& username,
            const std::string& passwordHash,
            const std::string& fullName
        );

        std::string getRole() const override;
    };
}

#endif
