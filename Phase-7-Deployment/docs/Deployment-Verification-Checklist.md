# Deployment Verification Checklist

Use this checklist on the actual target computer. Tick items only after performing them and recording evidence.

| Check | Expected outcome | Status |
|---|---|---|
| Source retrieved from intended Git commit | Correct project version is present | Not yet recorded |
| Required compiler and dependencies available | CMake configuration succeeds | Not yet recorded |
| Application builds | Build exits successfully | Not yet recorded |
| CTest executed | Test results saved; previously recorded baseline was 12/12 passing | Not yet recorded for deployment run |
| Database backup created (existing installation) | Recoverable copy exists | Not yet recorded |
| Application launched from project root | Correct root-level `lms.db` is used | Not yet recorded |
| First-run Administrator setup checked (fresh database) | Administrator can be created | Not yet recorded |
| Role smoke tests performed | Administrator, Lecturer, HoD and Student workflows open and behave as expected | Not yet recorded |
| Database persistence checked after restart | Records remain available | Not yet recorded |
| Backup/restore procedure checked | Backup can be restored | Not yet recorded |

Record date, tester, OS/toolchain, commit SHA, test output and any defects with the completed checklist.
