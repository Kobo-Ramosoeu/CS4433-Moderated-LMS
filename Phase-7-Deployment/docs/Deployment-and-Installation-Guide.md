# Moderated LMS — Deployment and Installation Guide

## 1. System overview
Moderated LMS is a local/offline C++17 console application. It uses SQLite for persistent storage, Argon2id for password hashing, and Windows BCrypt for secure salt generation.

## 2. Supported build environment
The verified development environment recorded for this project is:
- Windows
- MSYS2 UCRT64 toolchain (G++)
- CMake
- SQLite development package
- Argon2 development package
- Git

This is a developer/deployment guide for the verified Windows environment, not a claim of tested support on Linux or macOS.

## 3. Obtain the source
Clone the repository or download its source from GitHub. Open PowerShell in the repository root.

## 4. Configure and build
The project README records the following commands (adjust the CMake executable and MSYS2 path if installed elsewhere):

```powershell
& "C:\Program Files\CMake\bin\cmake.exe" -S . -B build-phase4 -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="C:/msys64/ucrt64"
& "C:\Program Files\CMake\bin\cmake.exe" --build build-phase4
```

If CMake or a dependency cannot be found, verify that the MSYS2 UCRT64 packages are installed and that the configured prefix matches the actual MSYS2 location.

## 5. Run automated tests
```powershell
ctest --test-dir build-phase4 --output-on-failure
```

The project has a previously recorded result of 12/12 tests passing. Run the tests again on the target machine and retain the current output as deployment evidence.

## 6. First launch
Run the application from the repository root:

```powershell
.\build-phase4\lms.exe
```

The application uses `lms.db` relative to the current working directory. On a database with no Administrator account, first-run setup prompts creation of an Administrator. Protect the Administrator credentials.

## 7. Existing database and backup
The source package intentionally excludes the local database. Before updating an existing installation:
1. Close the application.
2. Make a separate backup copy of `lms.db`.
3. Keep the existing database in the project root.
4. Build and launch the updated application from the project root.
5. Verify the application starts and the expected records remain available.

Never overwrite a real academic database with a test database. Test using a disposable copy.

## 8. Security and operational notes
- Use unique accounts and strong passwords.
- Do not share administrator credentials.
- Restrict access to the machine and database file to authorised users.
- Keep independent backups of the database.
- Do not store passwords or secrets in Git.
- Use only trusted local file paths when submitting assignment files.

## 9. Troubleshooting
| Symptom | Checks |
|---|---|
| Executable not found | Confirm build succeeded and `build-phase4\lms.exe` exists. |
| Database appears empty or wrong | Confirm current directory is the repository root; the app opens `lms.db` relative to it. |
| CMake cannot find a dependency | Check MSYS2 UCRT64 installation and `CMAKE_PREFIX_PATH`. |
| Login fails | Check account role, username, password and active status; use Administrator account management where appropriate. |
| Build directory is stale | Reconfigure from source; only remove a generated build directory when necessary, never the database. |

## 10. Deployment acceptance
A deployment is accepted only after a fresh build, automated test run, first-run or existing-database launch check, and role-based smoke tests have been performed and recorded. This guide does not claim those checks have been completed on every target machine.
