# Test Plan — Moderated LMS

## 1. Objective

The objective of this test plan is to verify that the Moderated Learning Management System (LMS) satisfies its functional requirements, business rules, security constraints, and expected workflows.

Testing focuses on deadline calculations, submissions, extensions, marking, moderation, mark release, and account management.

The plan covers unit, integration, system, acceptance, and regression testing. Automated test results are distinguished from interactive workflows that have not yet been manually verified.

## 2. Test Strategy and V-Model Mapping

The project follows a verification and validation approach based on the V-model. Each development level is associated with an appropriate testing level.

| Development Artefact | Corresponding Test Level | Verification Focus |
|---|---|---|
| Requirements and use-case specifications | Acceptance testing | Whether the system satisfies user and stakeholder requirements |
| System architecture and workflow design | System testing | Whether complete system workflows operate correctly |
| Component and repository design | Integration testing | Whether repositories and components interact correctly |
| Detailed algorithms and business rules | Unit testing | Whether individual functions and business rules produce correct results |

### 2.1 Test Levels

| Level | Scope | Entry Criteria | Exit Criteria |
|---|---|---|---|
| Unit | Timestamp parsing, deadline calculations, late-penalty calculations, and individual business rules | Relevant source files compile and test cases are defined | Defined unit tests pass and boundary conditions are checked |
| Integration | Assignment, extension, submission, marking, moderation, and administration repositories | SQLite schema is initialized in a disposable test database | Tested operations produce expected database changes and invalid operations are rejected |
| System | Student, Lecturer, HoD, and Administrator workflows | Application builds and test data is available | Required system workflows and access boundaries are verified |
| Acceptance | User requirements and use-case scenarios | System testing is completed and the system is available for evaluation | Required acceptance scenarios are demonstrated and evaluated |
| Regression | Previously working functionality affected by changes | Relevant changes are integrated | Registered regression tests pass |

### 2.2 Entry and Exit Criteria

**General entry criteria:**
- The relevant implementation is available.
- The application and required test targets compile.
- Test data and a suitable test database are available.
- Test cases and expected outcomes are defined.

**General exit criteria:**
- All required tests have been executed.
- Test outcomes have been recorded.
- Critical defects affecting required functionality have been addressed or documented.
- Test results and coverage evidence are retained.
- Any functionality not tested is clearly identified rather than reported as verified.

## 3. Test Design Techniques

The following techniques are used to select meaningful test inputs and identify errors.

### 3.1 Equivalence Partitioning

Inputs are divided into groups expected to produce similar system behaviour. Representative values are selected from each group.

Examples include:

| Functionality | Equivalence Classes |
|---|---|
| Submission timing | On time, within the permitted late period, beyond the 72-hour limit |
| Mark entry | Valid mark, negative mark, mark exceeding maximum |
| Moderation decision | Valid decision, invalid decision |
| Account status | Active account, inactive account |
| Deadline extension | Valid extension, invalid extension |

### 3.2 Boundary Value Analysis

Tests concentrate on values at, immediately before, and immediately after important boundaries.

Examples include:
- Submission exactly at the deadline.
- Submission just before and after the deadline.
- Submission at 24, 48, and 72 hours late.
- Submission beyond the 72-hour limit.
- Marks at zero and at the maximum permitted mark.

The deadline test cases in Section 4 provide examples of this technique.

## 4. Test Cases

| ID | Scenario | Expected Result |
|---|---|---|
| DL-01 | Submission exactly at the deadline | Zero late days and no penalty |
| DL-02 | Submission 12 hours late | Zero completed late days and no penalty |
| DL-03 | Submission exactly 24 hours late | One late day and a deduction of 10% of the maximum mark |
| DL-04 | Submission 36 hours late | One completed late day and a deduction of 10% |
| DL-05 | Submission exactly 48 hours late | Two late days and a deduction of 20% |
| DL-06 | Submission exactly 72 hours late | Three late days and a deduction of 30% |
| DL-07 | First submission more than 72 hours late | New submission rejected; historical penalty calculation returns zero |
| DL-08 | Impossible date or invalid timestamp | Validation fails and the invalid value is not saved |
| DL-09 | Legacy date-only deadline | Interpreted as 23:59:59 local time |
| EX-01 | Individual deadline extension | Only the selected student's effective deadline is affected by that individual extension |
| EX-02 | Whole-class deadline extension | Enrolled students receive the applicable class extension |
| EX-03 | Individual and class extensions both apply | The later applicable deadline is used |
| SU-01 | Resubmission before or on the effective deadline | A new attempt is recorded with the next attempt number |
| SU-02 | Resubmission after the effective deadline | Resubmission is rejected |
| SU-03 | Concurrent attempt allocation | Attempt numbers remain unique and sequential, with no partial records |
| MK-01 | Lecturer marks a submission for their own course | The permitted mark operation and associated audit record are saved |
| MK-02 | Lecturer attempts to mark another lecturer's submission | Operation is rejected |
| MD-01 | HoD attempts to moderate a mark outside their department | Operation is rejected |
| MD-02 | HoD adjusts a mark | Audit history records the relevant old and new raw and final mark values |
| MD-03 | Student views an unreleased mark | The unreleased mark remains hidden |
| AC-01 | Deactivate the last active Administrator | Operation is rejected |
| AC-02 | Assign an inactive HoD | Operation is rejected |

**Coverage note:** These cases define the intended verification scope. A case is only considered passed when its corresponding test has been executed and its expected result confirmed. A successful build or a related test suite does not independently prove that every listed scenario has passed.

## 5. Current Verification Evidence

### 5.1 Build and Automated Test Results

The application and test targets were successfully configured and built using the Windows/MSYS2 development environment.

The latest recorded CTest execution produced:

**12/12 tests passed.**

The automated test targets include:

- `deadline_calculator_unit_tests` — deadline calculation unit tests.
- `test_deadline` — deadline boundary tests.
- `repository_integration_tests` — repository integration tests.
- `submission_workflow_tests` — submission workflow tests.
- `mark_moderation_release_tests` — marking, moderation, and release tests.
- `database_tests` — database behaviour and error-handling tests.

The Catch2 unit-test target contributes multiple individual test cases to the CTest total.

The successful results confirm that all 12 registered CTest tests passed in the recorded run.

### 5.2 Code Coverage Results

The latest recorded coverage report was generated using gcovr.

| Source File | Lines | Executed | Coverage |
|---|---:|---:|---:|
| AdministrationRepository.cpp | 266 | 241 | 90% |
| Database.cpp | 56 | 47 | 83% |
| DeadlineExtensionRepository.cpp | 55 | 42 | 76% |
| MarkingRepository.cpp | 251 | 173 | 68% |
| ModerationRepository.cpp | 311 | 269 | 86% |
| SubmissionRepository.cpp | 105 | 80 | 76% |
| DeadlineCalculator.cpp | 28 | 28 | 100% |
| **TOTAL** | **1,072** | **880** | **82%** |

The measured overall line coverage is 82%, exceeding the project target of at least 70% for the reported source files.

Coverage is not uniform across all files. MarkingRepository.cpp has 68% coverage and remains an opportunity for further improvement.

**Coverage limitation:** The reported percentage applies to the seven source files included in the gcovr report. It should not be interpreted as coverage of every source file or every possible system workflow.

### 5.3 Repository and Workflow Evidence

The automated tests provide evidence for the following tested areas:

- Repository operations using an isolated SQLite test database.
- Submission attempts and deadline-related rejection behaviour.
- Marking and moderation workflows.
- Student mark visibility and release restrictions.
- Audit-history operations exercised by the tests.
- Database initialization, SQL execution, and selected error conditions.

These results apply to the specific scenarios exercised by the tests. They do not establish that every possible input, concurrency condition, or interactive role workflow has been tested.

### 5.4 Remaining Verification

The following activities require additional confirmation before claiming complete end-to-end acceptance:

- Manually exercise the required interactive workflows for Student, Lecturer, HoD, and Administrator roles.
- Compare every test-plan case with its corresponding test source and assertions.
- Confirm whether concurrent submission allocation has been independently tested.
- Confirm any audit-history requirements not directly asserted by the existing tests.
- Verify that the CI workflow automatically builds and runs tests on pushes.

## 6. Defect Logging

Defects identified during testing must be recorded and tracked until they are resolved or formally accepted as outstanding.

The defect log should contain:

| Field | Description |
|---|---|
| Defect ID | Unique identifier |
| Date | Date discovered |
| Test Case | Related test case or workflow |
| Description | Observed incorrect behaviour |
| Severity | Critical, High, Medium, or Low |
| Cause | Identified reason for the defect |
| Fix | Change made to address the defect |
| Retest Result | Pass, Fail, or Pending |
| Status | Open, Fixed, Retested, or Closed |

Only defects actually encountered should be recorded. If no defect was identified during a particular test run, this should be stated explicitly.

## 7. Test Data and Safety

All integration tests must use a disposable test database.

Destructive test setup must never be run against the user's existing `lms.db`.

An existing database must be backed up before a schema migration or any operation that could alter persistent data.

Test results must identify the test executed and its observed outcome. A requirement must not be marked as verified merely because a related source file compiles.

## 8. Registered Test Targets

| Test Target | Main Purpose | Latest Recorded Result |
|---|---|---|
| `deadline_calculator_unit_tests` | Deadline calculation unit tests | Included in 12/12 passing CTest run |
| `test_deadline` | Deadline and late-penalty boundary tests | PASS |
| `repository_integration_tests` | Repository integration scenarios | PASS |
| `submission_workflow_tests` | Submission and resubmission rules | PASS |
| `mark_moderation_release_tests` | Marking, moderation, visibility, and release workflows | PASS |
| `database_tests` | Database initialization, SQL execution, and error handling | PASS |

The executable target names and registered CTest names may differ.

## 9. Acceptance Summary

The latest recorded verification baseline is:

- Successful Windows/MSYS2 application and test build.
- 12 out of 12 registered CTest tests passed.
- 82% overall line coverage across the seven reported source files.
- Automated tests covering deadline calculations, repository operations, submission workflows, marking, moderation, release, and database behaviour.

These results provide evidence of successful compilation and automated regression testing.

They do not, by themselves, prove that every role workflow, security boundary, invalid-input scenario, concurrency condition, or acceptance requirement has been independently verified.

Phase 5 should be considered closed when the test evidence is retained, the outstanding verification items are resolved or explicitly documented, and the defect log is completed.