# Requirements Traceability Matrix

This matrix connects the system requirements to the relevant design elements, implementation files, and available verification evidence.

A passing test suite provides evidence only for the scenarios that it actually exercises. Requirements without confirmed, case-specific coverage must receive additional verification.

| Requirement | Design Element | Implementation | Verification Reference | Evidence Status |
|---|---|---|---|---|
| Authenticate active users | User repository and role-specific application workflows | `UserRepository.cpp`, `main.cpp` | Interactive authentication and role-access checks | Additional manual verification required |
| Allow students to view their enrolled courses and assignments | Enrolment and assignment repositories | `AssignmentRepository.cpp`, `AdministrationRepository.cpp` | Student workflow checks | Additional manual verification required |
| Submit and resubmit assignments | Submission workflow | `SubmissionRepository.cpp` | `submission_workflow` | Automated test suite passed |
| Enforce the effective submission deadline | Deadline calculation and extension repositories | `DeadlineCalculator.cpp`, `DeadlineExtensionRepository.cpp`, `SubmissionRepository.cpp`, `Timestamp.h` | `late_penalty_boundaries`, `repository_integration`, `submission_workflow` | Automated test suites passed; individual cases should be checked against test assertions |
| Calculate the late penalty at 10% of the maximum mark for each completed late day | Late-penalty calculation | `DeadlineCalculator.cpp` | `late_penalty_boundaries` | Automated test suite passed |
| Reject first submissions more than 72 hours late | Submission workflow and deadline policy | `SubmissionRepository.cpp`, `DeadlineCalculator.cpp` | `submission_workflow` | Automated test suite passed |
| Allow individual deadline extensions | Deadline extension repository | `DeadlineExtensionRepository.cpp`, `main.cpp` | `repository_integration` | Automated integration suite passed |
| Allow whole-class deadline extensions | Deadline extension model and lecturer workflow | `DeadlineExtensionRepository.cpp`, `main.cpp` | `repository_integration` | Automated integration suite passed |
| Apply the later applicable individual or class deadline | Effective-deadline calculation | `DeadlineExtensionRepository.cpp`, `DeadlineCalculator.cpp`, `SubmissionRepository.cpp` | `repository_integration`, `submission_workflow` | Automated test suites passed; verify the exact precedence assertions before claiming complete case coverage |
| Allocate attempt numbers and persist submissions safely | Submission repository and database transaction | `SubmissionRepository.cpp` | `submission_workflow` | Automated test suite passed; independent concurrency coverage still requires confirmation |
| Allow lecturers to mark submissions for their own courses | Marking repository | `MarkingRepository.cpp` | `mark_moderation_release` and lecturer access checks | Workflow suite passed; cross-lecturer access coverage requires confirmation |
| Restrict HoD moderation to the appropriate department | Moderation repository and department checks | `ModerationRepository.cpp` | HoD moderation tests and manual access checks | Additional case-specific verification required |
| Allow HoD review, adjustment, approval, and release of marks | Moderation and marking workflows | `ModerationRepository.cpp`, `MarkingRepository.cpp` | `mark_moderation_release` | Automated test suite passed |
| Keep unreleased marks hidden from students | Student mark-visibility rules | `MarkingRepository.cpp`, `ModerationRepository.cpp`, `main.cpp` | `mark_moderation_release` | Automated test suite passed for covered scenarios |
| Preserve audit history for marking and moderation | Audit records and database protections | `MarkingRepository.cpp`, `ModerationRepository.cpp`, `database/schema.sql` | `mark_moderation_release` and relevant audit assertions | Workflow suite passed; each required audit field should be confirmed against test assertions |
| Protect the last active Administrator from deactivation | Administration repository | `AdministrationRepository.cpp` | `repository_integration` | Automated integration suite passed |
| Reject assignment of an inactive HoD | Administration repository | `AdministrationRepository.cpp` | `repository_integration` | Automated integration suite passed |
| Support the implemented account and department management workflows | Administration repository | `AdministrationRepository.cpp`, `main.cpp` | Repository integration and manual administration workflow checks | Automated integration suite passed for covered cases; full interactive workflow requires confirmation |

## Verification Notes

1. The Windows/MSYS2 application build succeeded.
2. All four registered CTest tests passed: `late_penalty_boundaries`, `repository_integration`, `submission_workflow`, and `mark_moderation_release`.
3. Automated test success does not establish coverage for every requirement listed in this matrix.
4. Concurrency behavior, individual authorization boundaries, and specific audit-field assertions should be confirmed by inspecting the corresponding test cases or by adding targeted tests.
5. Manual verification of the complete Student, Lecturer, HoD, and Administrator workflows remains separate from the automated test results.

## Status Convention

- **Automated test suite passed:** The named test suite passed, but this label alone does not guarantee every related requirement has been tested.
- **Additional case-specific verification required:** Existing evidence does not yet establish that the exact requirement has been exercised and asserted.
- **Additional manual verification required:** The requirement needs an interactive check or other system-level evidence beyond the currently confirmed automated results.