# Defect Register — Moderated LMS

## 1. Purpose

This register records identified defects, the corrective actions implemented, and the available verification evidence.

A successful build confirms compilation but does not independently prove that a defect has been corrected. Verification status is therefore based on the available automated tests and manual checks.

## 2. Defect Register

| ID | Area | Identified Defect | Corrective Action | Verification Status |
|---|---|---|---|---|
| DL-01 | Deadline calculation | Incorrect handling of completed late-day boundaries | Corrected late-penalty calculation using completed 24-hour periods | Automated deadline regression suite passed |
| DL-02 | Submission deadline | First submissions could be accepted almost four days late | Added rejection of first submissions beyond 72 hours | Submission workflow suite passed |
| DL-03 | Assignment validation | Assignment deadlines accepted invalid non-empty strings | Improved timestamp and deadline validation | Build passed; exact repository validation assertions require confirmation |
| DL-04 | Deadline extensions | Extension date validation was insufficient | Improved deadline normalization and validation | Build passed; exact invalid-input assertions require confirmation |
| DL-05 | Submission timestamp | Submission processing used separate clock readings | Capture and reuse a single submission timestamp | Submission workflow suite passed; dedicated timing assertion requires confirmation |
| SU-01 | Submission attempts | Attempt allocation and insertion were not protected by one transaction | Added transaction-safe attempt allocation and submission insertion | Submission workflow suite passed; independent concurrency verification requires confirmation |
| EX-01 | Lecturer workflow | Lecturer menu supported individual extensions only | Added whole-class extension workflow | Repository integration suite passed; interactive menu verification remains necessary |
| MK-01 | Marking | Effective-deadline resolution was duplicated | Centralized effective-deadline calculation | Build and marking/moderation workflow suite passed; exact deadline-correction assertion requires confirmation |
| AUD-01 | Audit history | HoD mark adjustment audit lacked sufficient old/new mark information | Added old and new raw and final mark values to adjustment audit information | Marking/moderation workflow suite passed; exact field assertions require confirmation |
| AC-01 | Account administration | Last active Administrator could be deactivated | Added protection for the last active Administrator | Repository integration suite passed |
| AC-02 | Account administration | Inactive HoD could be assigned | Added active HoD validation | Repository integration suite passed |
| DOC-01 | Requirements | Phase 2 contains conflicting late-submission policy statements | Documented the conflict and the implementation decision | Documented; stakeholder confirmation remains necessary |

## 3. Verification Summary

The Windows/MSYS2 application build completed successfully.

All four registered CTest suites passed:

1. `late_penalty_boundaries`
2. `repository_integration`
3. `submission_workflow`
4. `mark_moderation_release`

The passing test suites provide automated regression evidence for their covered scenarios.

## 4. Outstanding Verification

The following checks should be completed before claiming full system acceptance:

- Manually test the interactive Student, Lecturer, HoD, and Administrator workflows.
- Confirm exact invalid timestamp and extension validation assertions.
- Verify submission timestamp consistency through a dedicated assertion if required.
- Run an independent concurrent submission test to demonstrate attempt allocation under concurrent requests.
- Confirm that marking and moderation audit tests assert every required old and new mark field.
- Obtain stakeholder clarification on the conflicting late-submission policy.

## 5. Status Definitions

| Status | Meaning |
|---|---|
| Corrected — Automated Evidence | Correction has been implemented and the relevant automated test suite has passed |
| Corrected — Partial Evidence | Correction has been implemented, but a dedicated assertion or additional verification is still needed |
| Pending Verification | The correction or requirement still needs the specified verification |
| Policy Clarification Required | A requirements conflict needs a stakeholder decision |

## 6. Overall Status

The identified corrections have been integrated into the active source tree. The application builds successfully in Windows/MSYS2, and all four registered automated test suites pass.

The remaining work concerns additional case-specific and interactive verification, rather than a currently reported failure in the registered test suites.