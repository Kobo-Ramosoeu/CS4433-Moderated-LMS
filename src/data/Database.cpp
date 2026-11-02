#include "lms/Database.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace lms
{
    Database::Database(const std::string& filePath)
        : connection(nullptr)
    {
        int result = sqlite3_open(filePath.c_str(), &connection);

        if (result != SQLITE_OK)
        {
            std::string message =
                connection != nullptr
                    ? sqlite3_errmsg(connection)
                    : "Unable to allocate SQLite connection";

            if (connection != nullptr)
            {
                sqlite3_close(connection);
                connection = nullptr;
            }

            throw std::runtime_error(
                "Could not open database: " + message
            );
        }

        try
        {
            execute("PRAGMA foreign_keys = ON;");
        }
        catch (...)
        {
            sqlite3_close(connection);
            connection = nullptr;
            throw;
        }
    }

    Database::~Database()
    {
        if (connection != nullptr)
        {
            sqlite3_close(connection);
        }
    }

    void Database::execute(const std::string& sql)
    {
        char* errorMessage = nullptr;

        int result = sqlite3_exec(
            connection,
            sql.c_str(),
            nullptr,
            nullptr,
            &errorMessage
        );

        if (result != SQLITE_OK)
        {
            std::string message =
                errorMessage != nullptr
                    ? errorMessage
                    : sqlite3_errmsg(connection);

            sqlite3_free(errorMessage);

            throw std::runtime_error(
                "Database operation failed: " + message
            );
        }
    }

    void Database::executeFile(const std::string& filePath)
    {
        std::ifstream file(filePath);

        if (!file.is_open())
        {
            throw std::runtime_error(
                "Could not open SQL file: " + filePath
            );
        }

        std::ostringstream contents;
        contents << file.rdbuf();

        if (file.bad())
        {
            throw std::runtime_error(
                "Could not read SQL file: " + filePath
            );
        }

        execute(contents.str());
    }
    sqlite3* Database::getConnection() const
    {
        return connection;
    }
}
