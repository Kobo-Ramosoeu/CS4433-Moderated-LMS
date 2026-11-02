#ifndef LMS_DATABASE_H
#define LMS_DATABASE_H

#include <sqlite3.h>
#include <string>

namespace lms
{
    class Database
    {
    private:
        sqlite3* connection;

    public:
        explicit Database(const std::string& filePath);
        ~Database();

        Database(const Database&) = delete;
        Database& operator=(const Database&) = delete;

        void execute(const std::string& sql);
        void executeFile(const std::string& filePath);

        sqlite3* getConnection() const;
    };
}

#endif