
#include "lms/UserRepository.h"

#include <memory>
#include <stdexcept>

namespace lms
{
    UserRepository::UserRepository(Database& database)
        : database(database)
    {
    }

    bool UserRepository::usernameExists(
        const std::string& username
    )
    {
        sqlite3_stmt* rawStatement = nullptr;

        const char* sql =
            "SELECT 1 FROM \"User\" WHERE username = ? LIMIT 1;";

        int result = sqlite3_prepare_v2(
            database.getConnection(),
            sql,
            -1,
            &rawStatement,
            nullptr
        );

        if (result != SQLITE_OK)
        {
            throw std::runtime_error(
                "Could not prepare username query."
            );
        }

        std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>
            statement(rawStatement, sqlite3_finalize);

        result = sqlite3_bind_text(
            statement.get(),
            1,
            username.c_str(),
            -1,
            SQLITE_TRANSIENT
        );

        if (result != SQLITE_OK)
        {
            throw std::runtime_error(
                "Could not bind username."
            );
        }

        result = sqlite3_step(statement.get());

        if (result == SQLITE_ROW)
        {
            return true;
        }

        if (result == SQLITE_DONE)
        {
            return false;
        }

        throw std::runtime_error(
            "Could not check username."
        );
    }

    void UserRepository::createUser(
        const std::string& username,
        const std::string& passwordHash,
        const std::string& fullName,
        const std::string& role
    )
    {
        if (username.empty() || passwordHash.empty()
            || fullName.empty())
        {
            throw std::invalid_argument(
                "Username, password hash, and full name are required."
            );
        }

        if (role != "Student" && role != "Lecturer"
            && role != "HoD" && role != "Administrator")
        {
            throw std::invalid_argument("Invalid user role.");
        }

        sqlite3_stmt* rawStatement = nullptr;

        const char* sql =
            "INSERT INTO \"User\" "
            "(username, password_hash, full_name, role) "
            "VALUES (?, ?, ?, ?);";

        int result = sqlite3_prepare_v2(
            database.getConnection(),
            sql,
            -1,
            &rawStatement,
            nullptr
        );

        if (result != SQLITE_OK)
        {
            throw std::runtime_error(
                "Could not prepare user registration."
            );
        }

        std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>
            statement(rawStatement, sqlite3_finalize);

        const std::string* values[] = {
            &username, &passwordHash, &fullName, &role
        };

        for (int i = 0; i < 4; ++i)
        {
            result = sqlite3_bind_text(
                statement.get(),
                i + 1,
                values[i]->c_str(),
                -1,
                SQLITE_TRANSIENT
            );

            if (result != SQLITE_OK)
            {
                throw std::runtime_error(
                    "Could not bind user registration data."
                );
            }
        }

        result = sqlite3_step(statement.get());

        if (result != SQLITE_DONE)
        {
            throw std::runtime_error(
                "Could not create user: "
                + std::string(
                    sqlite3_errmsg(database.getConnection())
                )
            );
        }
    }

    

bool UserRepository::findLoginDetails(
    const std::string& username,
    LoginDetails& details)
{
    const char* sql =
        "SELECT user_id, username, password_hash, full_name, role "
        "FROM \"User\" WHERE username = ? AND is_active = 1;";

    sqlite3_stmt* rawStatement = nullptr;

    int result = sqlite3_prepare_v2(
        database.getConnection(),
        sql,
        -1,
        &rawStatement,
        nullptr
    );

    if (result != SQLITE_OK)
    {
        throw std::runtime_error(
            "Failed to prepare login query."
        );
    }

    std::unique_ptr<
        sqlite3_stmt,
        decltype(&sqlite3_finalize)
    > statement(rawStatement, sqlite3_finalize);

    result = sqlite3_bind_text(
        statement.get(),
        1,
        username.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    if (result != SQLITE_OK)
    {
        throw std::runtime_error(
            "Failed to bind username."
        );
    }

    result = sqlite3_step(statement.get());

    if (result == SQLITE_DONE)
    {
        return false;
    }

    if (result != SQLITE_ROW)
    {
        throw std::runtime_error(
            "Failed to retrieve login details."
        );
    }

    details.userId = sqlite3_column_int(
        statement.get(), 0
    );

    details.username = reinterpret_cast<const char*>(
        sqlite3_column_text(statement.get(), 1)
    );

    details.passwordHash = reinterpret_cast<const char*>(
        sqlite3_column_text(statement.get(), 2)
    );

    details.fullName = reinterpret_cast<const char*>(
        sqlite3_column_text(statement.get(), 3)
    );

    details.role = reinterpret_cast<const char*>(
        sqlite3_column_text(statement.get(), 4)
    );

    return true;
}
}