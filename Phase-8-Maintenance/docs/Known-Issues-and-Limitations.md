# Known Issues and Limitations

## 1. Specification ambiguity — late submissions
The requirements contain a conflict for a first submission more than 72 hours after the effective deadline. One statement indicates a zero mark, while the detailed Submit Assignment use case indicates rejection. The implementation follows the detailed use-case rule and rejects the new first submission. Lecturer/product-owner confirmation is required before changing this behaviour.

## 2. Manual acceptance testing
The recorded automated test run passed 12/12 tests, but Phase 5 and Phase 6 records explicitly state that interactive role-based acceptance scenarios remain to be executed and documented. In particular, whole-class extension visibility on the Student assignment list should be demonstrated end-to-end.

## 3. Platform scope
The current implementation is a local/offline Windows console application. Web access, mobile applications, online payments, video conferencing, automated grading and plagiarism detection are outside the documented project scope.

## 4. Deployment scope
The verified build environment is Windows with MSYS2 UCRT64, CMake, SQLite and Argon2. Support on other operating systems has not been established by the project evidence.

## 5. Code review scope
The Phase 6 code review was focused on eight files affected by static-analysis findings. It was not a complete manual review of every source file.

## 6. Coverage scope
The recorded 82% coverage applies to seven selected source files, not the entire application.

## 7. Defect records
The earlier defect log contained a placeholder rather than a complete verified list of defects. New defects should be recorded with reproducible steps, severity, status and resolution evidence. Do not interpret an empty or placeholder log as proof that no defects exist.
