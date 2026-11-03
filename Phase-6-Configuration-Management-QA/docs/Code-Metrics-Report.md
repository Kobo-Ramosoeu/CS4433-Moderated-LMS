\# Code Metrics Report



\## 1. Introduction



This report presents the code metrics collected for the Moderated Learning Management System (LMS) project during Phase 6: Configuration Management and Quality Assurance.



The metrics were collected using Sloc, Cloc and Code (scc) version 4.1.0.



\## 2. Measurement Scope



The primary measurement focused on the project's C++ implementation files:



\- `src/` — C++ source files.

\- `include/` — C++ header files.



The generated build directories and Git metadata were excluded.



A supplementary measurement was also performed on the entire repository, excluding `.git`, `build`, and `build-phase4`.



\## 3. C++ Source Code Metrics



| Metric | Result |

|---|---:|

| Total files | 34 |

| Total lines | 4,934 |

| Blank lines | 861 |

| Comment lines | 34 |

| Code lines | 4,039 |

| Complexity score | 604 |



\### Breakdown by language



| Language | Files | Lines | Code | Complexity |

|---|---:|---:|---:|---:|

| C++ | 17 | 4,249 | 3,501 | 577 |

| C++ Header | 17 | 685 | 538 | 27 |

| \*\*Total\*\* | \*\*34\*\* | \*\*4,934\*\* | \*\*4,039\*\* | \*\*604\*\* |



\## 4. Supplementary Repository Metrics



The entire repository measurement produced the following results:



| Metric | Result |

|---|---:|

| Total files | 71 |

| Total lines | 33,335 |

| Code lines | 31,229 |

| Complexity score | 803 |



This measurement includes supporting project files such as documentation, HTML reports, configuration files, and other resources. It is supplementary and is not used as the primary C++ implementation measurement.



\## 5. Cyclomatic Complexity Note



The complexity values in this report are the complexity scores reported by scc 4.1.0.



These values are tool-reported aggregate scores. They should not be interpreted as the average cyclomatic complexity per function or as the complexity of a single function.



\## 6. Tool Information



\- Tool: Sloc, Cloc and Code (scc)

\- Version: 4.1.0

\- Primary directories analysed: `src/` and `include/`

\- Excluded generated directories: `.git`, `build`, and `build-phase4`



\## 7. Conclusion



The C++ implementation consists of 34 source and header files containing 4,039 lines of code. The reported complexity score is 604.



These metrics provide a baseline for assessing the size and structural complexity of the LMS implementation.

