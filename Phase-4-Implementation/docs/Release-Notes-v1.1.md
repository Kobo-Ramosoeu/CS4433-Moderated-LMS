# Release Notes — Moderated LMS v1.1

## 1. Overview

Version 1.1 includes corrections and improvements to assignment deadline handling, submission processing, marking, moderation, audit records, and account administration.

The changes aim to improve consistency, data integrity, and enforcement of the LMS business rules.

## 2. Improvements and Corrections

### 2.1 Deadline Handling

- Added timestamp-based deadline handling.
- Legacy date-only deadlines are interpreted as 23:59:59 local time.
- Improved validation of deadline values.
- Added handling for individual and whole-class deadline extensions.
- The effective deadline considers applicable extensions.

### 2.2 Late Submission Policy

- A deduction of 10% of the assignment's maximum mark applies for each completed 24-hour period after the effective deadline.
- Submissions less than 24 hours late do not receive a completed-day penalty.
- First submissions more than 72 hours late are rejected.
- Historical penalty calculations for submissions beyond the 72-hour boundary return zero.

### 2.3 Submission Processing

- Improved submission attempt allocation.
- Submission insertion and attempt allocation are performed within a database transaction.
- The submission timestamp is captured once and reused during processing.
- Resubmissions are checked against the student's effective deadline.

### 2.4 Marking and Moderation

- Centralized effective-deadline resolution for marking and returned-mark corrections.
- Improved audit information for HoD mark adjustments.
- Audit records include the relevant old and new raw and final mark values.
- Existing marking, moderation, approval, and release workflows are covered by automated regression tests.

### 2.5 Account Administration

- Added protection against deactivating the last active Administrator.
- Improved validation when assigning a Head of Department.
- Inactive HoDs cannot be assigned through the validated administration workflow.

### 2.6 Lecturer Workflow

- Added whole-class deadline extension support.
- Student ID `0` represents a whole-class extension in the relevant extension workflow.

## 3. Compatibility and Database Considerations

- Existing database records are not automatically rewritten as part of these code changes.
- Back up the existing `lms.db` before applying database schema changes or migration procedures.
- Integration tests use disposable SQLite databases.
- Local database files and generated build files should not be included in the source release.

## 4. Known Policy Consideration

The Phase 2 requirements contain a policy inconsistency concerning submissions more than three days late.

The relevant business rule describes a zero mark after three days, while the use-case exception specifies rejection of a first submission after 72 hours.

The current implementation follows the use-case exception by rejecting new first submissions beyond 72 hours.

This policy should be confirmed with the relevant project stakeholders before a future release changes or finalizes the requirement.

## 5. Verification Results

The project was configured and built successfully in the Windows/MSYS2 development environment.

The following registered CTest suites passed:

| Test | Result |
|---|---|
| `late_penalty_boundaries` | PASS |
| `repository_integration` | PASS |
| `submission_workflow` | PASS |
| `mark_moderation_release` | PASS |
| **Total** | **4/4 PASS** |

The tests were executed using the project's configured CMake build directory.

These results confirm successful compilation and execution of the registered automated regression tests. They do not establish that every interactive role workflow, concurrency scenario, or security boundary has been manually verified.

## 6. Release Readiness

The application builds successfully, and all four registered automated test suites pass.

Before claiming complete system acceptance, the remaining interactive workflows and any requirements without specific automated test assertions should be verified.

**Release status:** Automated build and regression verification successful; further system-level acceptance verification remains necessary.