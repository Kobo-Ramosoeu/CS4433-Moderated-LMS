# Manual Acceptance Test Plan

## 1. Purpose
This plan defines the interactive checks needed in addition to the automated CTest suites. It is a test plan, not a claim that these tests have already passed.

## 2. Test environment
- Windows with MSYS2 UCRT64 toolchain
- C++17, CMake, SQLite and Argon2 dependencies
- Application launched from the repository root so that `lms.db` resolves correctly
- A disposable test database and test accounts for Administrator, Lecturer, Head of Department (HoD), and Student roles

## 3. Preconditions and safety
1. Back up any existing `lms.db`.
2. Use a disposable database for destructive or state-changing tests.
3. Create representative departments, courses, lecturer assignments, student enrolments and an assignment.
4. Record actual outcomes, date, tester, build/commit and any defect. Do not mark a case passed without executing it.

## 4. Acceptance scenarios

| ID | Role | Scenario | Expected result | Execution status |
|---|---|---|---|---|
| MAT-01 | Administrator | Create a Student and Lecturer account | Accounts and corresponding profiles are saved | Not yet evidenced in this report |
| MAT-02 | Administrator | Deactivate an account | Inactive account cannot authenticate | Not yet evidenced in this report |
| MAT-03 | Administrator | Attempt to deactivate the last active Administrator | Operation is rejected | Not yet evidenced in this report |
| MAT-04 | Administrator | Assign a HoD to a department | Valid active HoD is assigned | Not yet evidenced in this report |
| MAT-05 | Administrator | Enrol a student and assign a lecturer to a course | Course relationships are persisted | Not yet evidenced in this report |
| MAT-06 | Lecturer | Create an assignment for an assigned course | Assignment is saved and visible to enrolled students | Not yet evidenced in this report |
| MAT-07 | Student | Submit a file before the effective deadline | Submission is recorded with an attempt number and timestamp | Not yet evidenced in this report |
| MAT-08 | Student | Resubmit before the effective deadline | A new permitted attempt is recorded | Not yet evidenced in this report |
| MAT-09 | Lecturer | Grant an individual extension | Effective deadline changes for the selected student | Not yet evidenced in this report |
| MAT-10 | Lecturer | Grant a whole-class extension using student ID 0 | Effective deadline changes for the course/class scope | Not yet evidenced in this report |
| MAT-11 | Student | Submit within the allowed late window | Submission is accepted and applicable penalty is calculated | Not yet evidenced in this report |
| MAT-12 | Student | Submit a first attempt more than 72 hours late | Submission is rejected by the implemented detailed use-case rule | Not yet evidenced in this report |
| MAT-13 | Lecturer | Mark a submission and provide feedback | Mark and feedback are stored; status awaits moderation | Not yet evidenced in this report |
| MAT-14 | HoD | Return a mark for correction with a reason | Lecturer can identify the correction request | Not yet evidenced in this report |
| MAT-15 | Lecturer | Correct a returned mark | Updated mark is available for moderation | Not yet evidenced in this report |
| MAT-16 | HoD | Approve and release a mark | Student can view the mark only after release | Not yet evidenced in this report |
| MAT-17 | Student | Attempt to view an unreleased mark | Mark is not disclosed | Not yet evidenced in this report |
| MAT-18 | System | Inspect audit history after moderation/release | Relevant action and reason are recorded | Not yet evidenced in this report |

## 5. Automated verification already recorded
The project records four CTest suites: `late_penalty_boundaries`, `repository_integration`, `submission_workflow`, and `mark_moderation_release`. The previously recorded run passed 12/12 tests. This automated result does not replace the interactive acceptance scenarios above.

## 6. Results recording template
For each executed scenario record:
- Test ID and date/time
- Build/commit identifier
- Test account/role (never record passwords)
- Actual result and evidence location
- Pass / Fail / Blocked
- Defect ID, if applicable

## 7. Exit criteria
Acceptance testing is complete only when required scenarios have been executed, failures have been logged and resolved or formally accepted, and regression tests pass.
