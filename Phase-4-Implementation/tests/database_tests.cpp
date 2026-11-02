
#include "lms/Database.h"

#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

int main() {
    const char* dbPath = "database_tests.db";
    const char* sqlPath = "database_test_schema.sql";

    std::remove(dbPath);
    std::remove(sqlPath);

    try {
        {
            // Test 1: A database opens successfully.
            lms::Database db(dbPath);

            assert(db.getConnection() != nullptr);

            // Test 2: Foreign-key enforcement is enabled.
            sqlite3_stmt* statement = nullptr;

            int result = sqlite3_prepare_v2(
                db.getConnection(),
                "PRAGMA foreign_keys;",
                -1,
                &statement,
                nullptr
            );

            assert(result == SQLITE_OK);
            assert(sqlite3_step(statement) == SQLITE_ROW);
            assert(sqlite3_column_int(statement, 0) == 1);

            sqlite3_finalize(statement);

            // Test 3: Valid SQL executes successfully.
            db.execute(
                "CREATE TABLE TestTable ("
                "id INTEGER PRIMARY KEY,"
                "name TEXT NOT NULL);"
            );

            db.execute(
                "INSERT INTO TestTable(name) VALUES('Database test');"
            );

            // Test 4: Executed SQL changes the database.
            result = sqlite3_prepare_v2(
                db.getConnection(),
                "SELECT name FROM TestTable WHERE id = 1;",
                -1,
                &statement,
                nullptr
            );

            assert(result == SQLITE_OK);
            assert(sqlite3_step(statement) == SQLITE_ROW);

            const unsigned char* value =
                sqlite3_column_text(statement, 0);

            assert(value != nullptr);
            assert(std::string(
                reinterpret_cast<const char*>(value)
            ) == "Database test");

            sqlite3_finalize(statement);

            // Test 5: Invalid SQL throws a runtime_error.
            bool invalidSqlRejected = false;

            try {
                db.execute("THIS IS NOT VALID SQL;");
            }
            catch (const std::runtime_error&) {
                invalidSqlRejected = true;
            }

            assert(invalidSqlRejected);

            // Test 6: A valid SQL file can be loaded and executed.
            {
                std::ofstream file(sqlPath);

                assert(file.is_open());

                file << "CREATE TABLE FileTable ("
                     << "id INTEGER PRIMARY KEY,"
                     << "value TEXT);";

                file << "INSERT INTO FileTable(value) "
                     << "VALUES('Loaded from file');";
            }

            db.executeFile(sqlPath);

            // Verify the SQL file was executed.
            result = sqlite3_prepare_v2(
                db.getConnection(),
                "SELECT value FROM FileTable WHERE id = 1;",
                -1,
                &statement,
                nullptr
            );

            assert(result == SQLITE_OK);
            assert(sqlite3_step(statement) == SQLITE_ROW);

            value = sqlite3_column_text(statement, 0);

            assert(value != nullptr);
            assert(std::string(
                reinterpret_cast<const char*>(value)
            ) == "Loaded from file");

            sqlite3_finalize(statement);

            // Test 7: A nonexistent SQL file is rejected.
            bool missingFileRejected = false;

            try {
                db.executeFile("nonexistent_database_schema.sql");
            }
            catch (const std::runtime_error&) {
                missingFileRejected = true;
            }

            assert(missingFileRejected);

            // Test 8: SQL errors inside a file are also rejected.
            const char* invalidSqlPath = "invalid_database_test.sql";

            {
                std::ofstream file(invalidSqlPath);

                assert(file.is_open());
                file << "INVALID SQL STATEMENT;";
            }

            bool invalidFileSqlRejected = false;

            try {
                db.executeFile(invalidSqlPath);
            }
            catch (const std::runtime_error&) {
                invalidFileSqlRejected = true;
            }

            assert(invalidFileSqlRejected);

            std::remove(invalidSqlPath);

            // Test 9: Foreign-key constraints are enforced.
            db.execute(
                "CREATE TABLE ParentTable ("
                "id INTEGER PRIMARY KEY);"
            );

            db.execute(
                "CREATE TABLE ChildTable ("
                "id INTEGER PRIMARY KEY,"
                "parent_id INTEGER,"
                "FOREIGN KEY(parent_id) REFERENCES ParentTable(id));"
            );

            bool foreignKeyRejected = false;

            try {
                db.execute(
                    "INSERT INTO ChildTable(parent_id) VALUES(999);"
                );
            }
            catch (const std::runtime_error&) {
                foreignKeyRejected = true;
            }

            assert(foreignKeyRejected);

            // Test 10: An unavailable database path is rejected.
            bool invalidDatabasePathRejected = false;

            try {
                lms::Database invalidDb(
                    "nonexistent_database_folder/test.db"
                );
            }
            catch (const std::runtime_error&) {
                invalidDatabasePathRejected = true;
            }

            assert(invalidDatabasePathRejected);
        }

        // The database connection has been destroyed at this point.
        std::remove(dbPath);
        std::remove(sqlPath);

        std::cout << "Database tests passed.\n";
        return 0;
    }
    catch (const std::exception& error) {
        std::remove(dbPath);
        std::remove(sqlPath);

        std::cerr << "Database test failed: "
                  << error.what() << '\n';

        return 1;
    }
}
