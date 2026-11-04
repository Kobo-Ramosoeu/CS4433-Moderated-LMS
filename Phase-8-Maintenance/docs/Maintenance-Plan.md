# Moderated LMS — Maintenance Plan

## 1. Purpose
This plan describes how the LMS should be maintained after handover. It covers corrective, adaptive, perfective and preventive maintenance.

## 2. Maintenance categories
- Corrective: investigate and fix reproducible defects.
- Adaptive: update the application when supported Windows/toolchain/dependency environments change.
- Perfective: improve usability, reports or workflows without changing agreed business rules unexpectedly.
- Preventive: maintain backups, dependencies, tests, documentation and secure configuration to reduce future failures.

## 3. Issue handling workflow
1. Record the issue, reporter, date, role, steps to reproduce and expected/actual result.
2. Assign a unique issue/defect ID and severity.
3. Reproduce using a disposable database where possible.
4. Identify affected requirements, modules, schema and users.
5. Implement the fix on a feature branch.
6. Add or update a regression test.
7. Build and run CTest; perform relevant manual acceptance tests.
8. Update documentation and release notes.
9. Review and merge the change through the repository workflow.
10. Communicate the change and retain evidence.

## 4. Database backup and recovery
- Close the application before making a simple file-level backup of `lms.db`.
- Store backups separately from the working project directory.
- Restrict backup access because the database contains academic and account information.
- Retain multiple dated copies according to the institution's retention policy.
- Periodically test restoration using a separate copy.
- Never test restore procedures against the only copy of a live database.

## 5. Security maintenance
- Create individual accounts; do not share credentials.
- Remove or deactivate access when no longer authorised, while preserving required academic records.
- Keep compiler, CMake and dependencies from trusted sources and update deliberately.
- Review authentication, role checks, audit history and file-path handling when changing related code.
- Never commit passwords, production databases or private student information.

## 6. Regression and monitoring
For each change, run the available CTest suites:
- `late_penalty_boundaries`
- `repository_integration`
- `submission_workflow`
- `mark_moderation_release`

Add manual checks for affected role workflows. Review CI results and static-analysis output. Automated checks do not replace manual acceptance testing.

## 7. Maintenance schedule (recommended)
| Frequency / trigger | Activity |
|---|---|
| Before each release | Build, CTest, static analysis, review changes and update release notes |
| After each academic assessment period | Verify records and perform an authorised backup |
| Monthly or per institutional policy | Review account access and backup retention |
| After a reported defect | Reproduce, log, fix, regression-test and document |
| When dependencies change | Rebuild and rerun tests in the supported environment |

## 8. Ownership and limitations
The institution or designated system administrator should assign responsibility for account administration, database backups and access control. The project is currently a local console application; it does not provide a central hosted service or automated remote backup facility.
