#ifndef LMS_USER_H
#define LMS_USER_H

#include <string>

namespace lms {

class User {
private:
    int userId;
    std::string username;
    std::string passwordHash;
    std::string fullName;

public:
    User(
        int userId,
        const std::string& username,
        const std::string& passwordHash,
        const std::string& fullName
    );

    virtual ~User() = default;

    int getUserId() const;
    const std::string& getUsername() const;
    const std::string& getFullName() const;

    virtual std::string getRole() const = 0;
};

}

#endif