# Test Plan — Moderated LMS

## 1. Objective

Verify the LMS business rules and the corrected deadline, submission, marking, moderation, release, and account-management workflows.

The test plan covers unit tests, repository integration tests, regression tests, and system-level verification. Automated test results must be distinguished from interactive workflows that have not yet been manually verified.

## 2. Test Levels

| Level | Scope | Entry Criteria | Exit Criteria |
|---|---|---|---|
| Unit | Timestamp parsing and late-penalty calculation | Relevant source files compile | Defined timestamp and penalty boundary tests pass |
| Repository/Integration | Assignment, extension, submission, marking, moderation, and administration repositories | SQLite schema initialized in a disposable test database | Tested operations produce expected database changes; tested invalid operations are rejected |
| System | Student, Lecturer, HoD, and Administrator workflows | Application builds and test data is available | Required role workflows and access boundaries are verified |
| Regression | Previously working functions affected by corrections | Relevant changes integrated | Registered regression tests pass |

## 3. Test Cases

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

**Coverage note:** The cases above define the intended verification scope. A listed test case must not be treated as passed solely because the application builds or a related test suite passes. The evidence section below records the results confirmed so far.

## 4. Current Verification Evidence

### 4.1 Build and Automated Tests

- Windows/MSYS2 application build: **PASS**.
- CTest regression suite: **PASS — 4/4 registered tests**.
- `late_penalty_boundaries`: **PASS**.
- `repository_integration`: **PASS**.
- `submission_workflow`: **PASS**.
- `mark_moderation_release`: **PASS**.

The successful build confirms that the configured application and test targets compile in the Windows/MSYS2 environment. The four passing test suites provide automated regression evidence for the behavior covered by their respective test cases.

### 4.2 Repository and Workflow Evidence

- Repository integration tests ran against an isolated SQLite database.
- Submission workflow tests cover first submissions within and beyond the 72-hour boundary, attempt numbering, multiple attempts, and rejection of post-deadline resubmissions.
- Marking, moderation, and release tests cover the tested sequence of marking, student visibility restrictions, HoD review, lecturer correction, approval, release, and audit history.
- Whole-class extension and account-management checks are included in the repository integration coverage reported during development.

These results apply to the scenarios exercised by the tests. They do not establish that every possible input, concurrency condition, or interactive role workflow has been tested.

### 4.3 Remaining Verification

- Manually exercise all required interactive workflows through the Windows/MSYS2 application.
- Confirm the behavior of every test-plan case against the corresponding test source and assertions.
- Run a dedicated concurrency test if concurrent submission behavior must be demonstrated independently.
- Confirm any remaining audit-history requirements that are not directly asserted by the existing tests.

**Status:** The Windows/MSYS2 build and all four registered CTest suites have passed. Additional manual and case-specific verification remains necessary before claiming complete end-to-end acceptance.

## 5. Test Data and Safety

All integration tests must use a disposable test database. Never run destructive test setup against the user's existing `lms.db`.

Back up any existing database before a schema migration or other operation that could alter persistent data.

Test results must identify the test executed and its observed outcome. Do not mark a requirement as verified merely because a related source file compiles.

## 6. Registered Test Targets

| Test Target | Main Purpose | Confirmed Result |
|---|---|---|
| `test_deadline` / `late_penalty_boundaries` | Deadline and late-penalty boundary regression tests | PASS |
| `repository_integration_tests` / `repository_integration` | Repository integration scenarios, including the reported extension and account-management checks | PASS |
| `submission_workflow_tests` / `submission_workflow` | First-submission boundaries, attempt numbering, multiple attempts, and resubmission rules | PASS |
| `mark_moderation_release_tests` / `mark_moderation_release` | Marking, moderation, student visibility, release, and audit-history workflow | PASS |

The executable and CTest names may differ; the CTest names shown in the final column are the registered test names reported by the project.

## 7. Acceptance Summary

The current verification baseline is a successful Windows/MSYS2 build and four passing CTest tests.

This is evidence of successful compilation and automated regression testing. It is not, by itself, proof that every role workflow, security boundary, invalid-input case, or concurrency scenario has been manually or independently verified.