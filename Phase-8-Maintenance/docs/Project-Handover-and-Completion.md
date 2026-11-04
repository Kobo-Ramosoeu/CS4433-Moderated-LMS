# Project Handover and Completion Record

## 1. System delivered
The Moderated Learning Management System is a C++17 local/offline console application using SQLite. The documented roles are Administrator, Lecturer, Head of Department and Student.

## 2. Handover contents
The repository should contain:
- Source code, headers and CMake configuration
- Database schema and migration/initialisation logic
- Automated tests and CI workflow
- Phase 1–6 project records
- Phase 7 deployment and user documentation
- Phase 8 maintenance and known-issues documentation
- Design diagrams in the project images folder
- Final project report
- Presentation recording

The populated local database is not distributed as a standard source artefact. A deployment administrator must protect and back it up separately.

## 3. Verification evidence available
Project records report:
- Successful CMake build in the Windows/MSYS2 UCRT64 environment
- 12/12 automated CTest tests passing
- 82% coverage across seven selected source files
- Cppcheck final scan with zero diagnostics in the enabled categories across 17 source files
- Focused review of the eight files changed in response to static-analysis findings

These are historical project results. A fresh verification run should be performed on the final merged commit and retained as release evidence.

## 4. Outstanding acceptance conditions
The following items should be closed before declaring operational acceptance:
1. Execute and record the manual role-based acceptance test plan.
2. Verify whole-class extension visibility in the Student assignment list.
3. Confirm the late-submission policy conflict with the lecturer.
4. Run a fresh build and CTest on the final commit.
5. Complete a backup and restore check on a disposable database.
6. Confirm that the final report, diagrams, manuals and presentation recording are present in the submission repository.

## 5. Completion statement
This document records the handover structure and known verification conditions. It does not certify that all outstanding acceptance conditions have passed. Final operational acceptance should be recorded by the responsible lecturer or system owner after reviewing the evidence.
