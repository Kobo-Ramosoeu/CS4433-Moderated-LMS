
# Phase 4 – Implementation and Verification

This folder records the implementation baseline, change-request work, test strategy and verification evidence for the Moderated LMS.

## Implemented corrections in this update

- Shared local date/time parser and validator (`include/lms/Timestamp.h`).
- Deadline normalization to `YYYY-MM-DD HH:MM:SS`; legacy date-only deadlines are interpreted as 23:59:59.
- Late penalties count completed 24-hour periods (10% of assignment maximum per full period).
- First submissions are rejected when more than 72 hours past the effective deadline; resubmissions remain limited to the effective deadline.
- Submission time is captured once and reused for validation and persistence.
- Submission attempt allocation and insertion use `BEGIN IMMEDIATE`.
- Individual and whole-class extensions are available through the lecturer extension workflow (`student ID 0` means whole class).
- Marking, returned-mark correction and HoD adjustment share effective-deadline resolution.
- HoD adjustment audit values record both raw and final marks.
- Prevent deactivation of the last active Administrator and assignment of inactive HoD accounts.

## Verification status

The complete Windows application was successfully configured and built using CMake, the MSYS2 UCRT64 toolchain, SQLite and Argon2.

All four registered CTest suites passed (4/4):

- `late_penalty_boundaries`
- `repository_integration`
- `submission_workflow`
- `mark_moderation_release`

These results confirm successful compilation/linking and passing automated regression tests. They do not establish that every interactive role workflow has been manually tested.

Run the tests from PowerShell at the project root:

```powershell
ctest --test-dir build-phase4 --output-on-failure
```

## Additional workflow regression tests

The CTest suite includes disposable-database tests for:

- Deadline and late-penalty boundaries (`late_penalty_boundaries`).
- Class/individual extensions and account safeguards (`repository_integration`).
- First submissions, the 72-hour late window, resubmission eligibility and attempt numbering (`submission_workflow`).
- Lecturer marking, HoD return/correction/approval, release, student visibility and audit history (`mark_moderation_release`).

The workflow tests create temporary SQLite databases in the source working directory and remove them after closing the database connection. They do not use the user's production database.

See `docs/` for the test plan, defect register, traceability matrix and release notes.
