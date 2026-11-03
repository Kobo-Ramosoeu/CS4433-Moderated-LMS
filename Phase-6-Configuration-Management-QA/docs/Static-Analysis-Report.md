\# Static Analysis Report



\## 1. Introduction



Static analysis was performed on the Moderated LMS source code using Cppcheck to identify potential programming errors, performance improvements, and portability concerns.



The purpose of the analysis was to improve code quality, identify possible defects, and address the findings before final project integration.



\## 2. Analysis Tool



| Item | Details |

|---|---|

| Tool | Cppcheck |

| Version | 2.21.0 |

| Programming language | C++ |

| Source directory | `src/` |

| Include directory | `include/` |

| Files analysed | 17 |



The following command was used:



```bash

cppcheck \\

&#x20; --enable=warning,performance,portability \\

&#x20; --inline-suppr \\

&#x20; --suppress=missingIncludeSystem \\

&#x20; --error-exitcode=0 \\

&#x20; -I include \\

&#x20; src

```



\## 3. Initial Analysis Results



The initial analysis identified 32 findings across 17 source files.



| Finding category | Count |

|---|---:|

| Uninitialised member variable warnings | 27 |

| Performance suggestions (`returnByReference`) | 5 |

| \*\*Total\*\* | \*\*32\*\* |



\### 3.1 Uninitialised Member Variables



Several repository data structures contained numeric members without default initialisers.



Examples included:



\- `AdminUserRow`

\- `DepartmentRow`

\- `CourseRow`

\- `ReleasedMarkRow`

\- `CourseOption`

\- `SubmissionToMark`

\- `StudentReleasedMark`

\- `ReturnedMark`

\- `MarkForModeration`



These members were updated with default initialisers such as `{}` to provide safe initial values when objects are default-constructed.



\### 3.2 Performance Suggestions



Cppcheck identified string getter methods that returned member variables by value.



The affected classes were:



\- `User`

\- `Assignment`



The getters were updated to return `const std::string\&` instead of creating string copies.



This reduces unnecessary copying while allowing callers to read the stored strings without modifying them.



\## 4. Corrective Actions



The following files were modified:



| File | Improvement |

|---|---|

| `include/lms/AdministrationRepository.h` | Added numeric member initialisers |

| `include/lms/AssignmentRepository.h` | Initialised `CourseOption::courseId` |

| `include/lms/MarkingRepository.h` | Initialised numeric structure members |

| `include/lms/ModerationRepository.h` | Initialised numeric structure members |

| `include/lms/User.h` | Changed string getter return types |

| `src/User.cpp` | Updated getter implementations |

| `include/lms/models/Assignment.h` | Changed string getter return types |

| `src/models/Assignment.cpp` | Updated getter implementations |



The numeric members of `MarkingResult` were also given default initialisers as a defensive improvement.



\## 5. Final Analysis Results



Cppcheck was executed again after the corrective changes.



| Metric | Initial analysis | Final analysis |

|---|---:|---:|

| Files checked | 17 | 17 |

| Uninitialised member warnings | 27 | 0 |

| Performance suggestions | 5 | 0 |

| Total findings | 32 | 0 |



The final output contained no warning, performance, or error diagnostics.



\## 6. Build and Test Verification



After the code improvements, the project was rebuilt successfully using CMake.



Automated tests were executed using CTest.



| Verification | Result |

|---|---|

| Project build | Successful |

| Automated tests passed | 12 |

| Automated tests failed | 0 |

| Total automated tests | 12 |



All 12 automated tests passed.



\## 7. Evidence Files



The following files preserve the analysis results:



- `Phase-6-Configuration-Management-QA/docs/evidence/cppcheck-output.txt` — initial Cppcheck output.
- `Phase-6-Configuration-Management-QA/docs/evidence/cppcheck-final-output.txt` — final Cppcheck output.


Both files are stored in the project root directory.



\## 8. Conclusion



The static analysis process identified 32 findings in the Moderated LMS source code. Corrective changes were made to initialise numeric data members and improve string getter performance.



The final analysis reported zero remaining diagnostics across the 17 checked source files. The project also built successfully, and all 12 automated tests passed.



These results provide evidence that static analysis and corrective code improvements were performed as part of the project's quality assurance activities.

