# Release Management and Change Control

## 1. Purpose
This document defines how stable project versions are identified and how changes are integrated.

## 2. Branching and integration
- `main` is the stable integration branch.
- Phase work is performed on dedicated branches.
- Changes are built and tested before integration.
- Pull Requests provide a review and CI record.
- Do not force-push shared branches or commit generated build directories/database files.

## 3. Versioning
Use Semantic Versioning (`MAJOR.MINOR.PATCH`):
- MAJOR: incompatible system changes.
- MINOR: backward-compatible functionality.
- PATCH: backward-compatible fixes.

A tag should only be created after the corresponding commit is on `main`, build and automated tests have passed, and release notes are reviewed. No release tag is asserted by this document.

## 4. Change control
Each change should identify:
1. Request and reason
2. Affected requirements, design, code, database and tests
3. Risk and backward-compatibility considerations
4. Implementation branch and commits
5. Build/test evidence
6. Review and merge decision
7. Documentation and release-note updates

## 5. Configuration items
Controlled items include source/header files, CMake configuration, database schema/migrations, automated tests, CI workflow, project documentation, diagrams and release notes. Local databases, credentials, build outputs and temporary files must not be committed.

## 6. Current evidence and limitations
The repository contains Phase 5 verification records and Phase 6 configuration-management, static-analysis, metrics and focused code-review records. The recorded Cppcheck final scan reported zero diagnostics for the enabled checks; the recorded CTest run passed 12/12. The focused code review did not cover every source file, and interactive acceptance testing remains a separate activity.
