
# CS4433 – Moderated Learning Management System

A local/offline C++17 console application for managing courses, assignment submissions, marking, moderation and controlled release of marks.

## Technology

- C++17
- CMake
- SQLite
- Argon2id password hashing
- Windows BCrypt for secure salt generation

## Build (Windows / MSYS2 UCRT64)

The Windows application uses SQLite, Argon2, and Windows BCrypt. CMake searches for Argon2 under `C:/msys64/ucrt64` by default; change `MSYS2_ROOT` if your MSYS2 installation is elsewhere. SQLite is discovered through CMake.

```powershell
& "C:\Program Files\CMake\bin\cmake.exe" -S . -B build-phase4 -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="C:/msys64/ucrt64"
& "C:\Program Files\CMake\bin\cmake.exe" --build build-phase4
ctest --test-dir build-phase4 --output-on-failure
.\build-phase4\lms.exe
```

If `build-phase4` already exists, CMake normally reconfigures when `CMakeLists.txt` changes. If configuration becomes stale, remove only the build directory and configure again.

## First run

The application uses `lms.db` in the project root. If the database contains no Administrator account, the application runs a first-time setup prompt to create one. After that, only an Administrator can create and manage accounts. Student and Lecturer accounts are created with their corresponding Student/Lecturer profile records in one database transaction.

**Database safety:** the source package intentionally does not include a database file. If updating an existing installation, keep your current root-level `lms.db`. Back it up before the first run of this version. The application creates missing tables and applies a backward-compatible `is_active` account migration.

## Role workflows

### Administrator
- Create accounts for Students, Lecturers, HoDs and Administrators.
- List, update and activate/deactivate user accounts.
- Create, list and rename departments.
- Assign a HoD to a department.
- Create, list and update courses.
- Assign lecturers to courses.
- Enrol students in courses.

### Lecturer
- Create and view assignments for assigned courses.
- Review unmarked submissions and enter marks with mandatory written feedback.
- Grant individual student or whole-class deadline extensions with a reason. Enter student ID `0` for a whole-class extension.
- Review and correct marks returned by the HoD.

### Head of Department
- Review marks awaiting moderation for the HoD's department.
- Approve marks or return them to the Lecturer with a mandatory reason.
- Adjust a mark during moderation with a mandatory reason; adjustments are recalculated using the late-penalty rule and audited.
- Release approved marks to students. Release is recorded in the audit log.

### Student
- View enrolled courses and assignments for those courses.
- Submit work using a local file path.
- Resubmit before the effective deadline.
- Submit an initial late submission within 72 hours after the effective deadline.
- View only their own marks after HoD release, including feedback.

## Business rules implemented

- Students cannot view marks before HoD release.
- The late penalty is 10% of the assignment maximum mark for each completed 24-hour period, up to 72 hours. New first submissions more than 72 hours late are rejected.
- Individual and whole-class extensions change the effective deadline for the applicable students.
- Resubmissions are allowed before the effective deadline.
- Audit records are append-only through database triggers that reject normal UPDATE and DELETE operations.

## Database location

`main.cpp` opens `lms.db` relative to the current working directory. Run the application from the project root so it uses the intended database.

## Verification status

The application was successfully configured and built on Windows using CMake, the MSYS2 UCRT64 toolchain, SQLite and Argon2.

All four registered CTest suites passed:

- `late_penalty_boundaries`
- `repository_integration`
- `submission_workflow`
- `mark_moderation_release`

Repeat the tests from the project root using:

```powershell
ctest --test-dir build-phase4 --output-on-failure
```

The successful build and automated tests do not, by themselves, establish that every interactive role workflow has been manually tested.


## Project phases and supporting documentation

- [Phase 1 — Project Planning](Phase-1-Project-Planning/README.md)
- [Phase 2 — Requirements Engineering](Phase-2-Requirements-Engineering/README.md)
- [Phase 3 — System Design](Phase-3-System-Design/README.md)
- [Phase 4 — Implementation](Phase-4-Implementation/README.md)
- [Phase 5 — Verification and Validation](Phase-5-Verification-Validation/)
- [Phase 6 — Configuration Management and QA](Phase-6-Configuration-Management-QA/README.md)
- [Phase 7 — Deployment](Phase-7-Deployment/README.md)
- [Phase 8 — Maintenance and Handover](Phase-8-Maintenance/README.md)

## Final submission artefacts

The final submission report, project diagrams and presentation recording should be stored in the repository submission folder/root as required by the course instructions. The recording is named `LMS Presentation.mp4`. Verify that the report PDF, diagram image files and recording are visible on the GitHub `main` branch before submitting the repository link.

## Known acceptance conditions

The recorded automated baseline is 12/12 CTest tests passed. Manual role-based acceptance checks, deployment verification and the whole-class extension visibility check must still be executed and recorded before claiming full operational acceptance. The specification conflict for first submissions more than 72 hours late is documented in Phase 8.
