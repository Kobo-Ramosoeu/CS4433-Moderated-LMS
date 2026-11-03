\# Configuration Management Strategy



\## 1. Introduction



This document describes the configuration management practices used during the development of the Moderated LMS. These practices help maintain code consistency, track changes, support collaboration, and ensure that the project remains stable.



\## 2. Git Branching Strategy



The project uses a feature-branch workflow.



| Branch | Purpose |

|---|---|

| `main` | Contains the stable, integrated version of the project. |

| `phase-5-verification-validation` | Used for Phase 5 verification and validation activities. |

| `phase-6-configuration-management-qa` | Used for Phase 6 configuration management and quality assurance activities. |



New development work is performed on a separate branch rather than directly on `main`. Once the work is completed and tested, it is submitted for review through a GitHub Pull Request.



\## 3. Commit Management



Commits should be small, meaningful, and describe the changes being introduced.



Examples of commit messages include:



\- `Implement and test Moderated LMS Phase 4`

\- `Complete Phase 5 verification and validation`

\- `Add Phase 6 configuration management documentation`

\- `Add static analysis and code metrics reports`



This approach makes it easier to identify, review, and track project changes.



\## 4. Pull Request and Integration Process



The following process is used when integrating completed work:



1\. Create a new branch from the latest `main`.

2\. Implement the required changes on that branch.

3\. Build the project and run the automated tests.

4\. Push the branch to GitHub.

5\. Create a Pull Request targeting `main`.

6\. Review the proposed changes and check the CI results.

7\. Merge the Pull Request after successful checks.



Direct development commits to `main` are avoided.



\## 5. Continuous Integration



GitHub Actions is used to automate the project's build and testing process.



The CI workflow is configured to run when changes are pushed to branches and when Pull Requests are created against `main`.



The workflow performs the following activities:



1\. Checks out the repository.

2\. Sets up the MSYS2 UCRT64 development environment.

3\. Configures the project using CMake.

4\. Builds the application and test targets.

5\. Runs automated tests using CTest.



A successful CI run provides evidence that the changes can be built and that the automated tests pass.



\## 6. Release and Tag Management



Git tags are used to identify important project milestones and stable releases.



Release tags will follow Semantic Versioning:



`vMAJOR.MINOR.PATCH`



\- \*\*MAJOR:\*\* Significant changes that introduce incompatible functionality.

\- \*\*MINOR:\*\* New functionality that remains compatible with the existing system.

\- \*\*PATCH:\*\* Bug fixes and minor corrections.



Release tags will be created on stable commits after the relevant work has been reviewed, tested, and merged into `main`.



\## 7. Configuration Management Practices



The project follows these practices:



\- Source code and project documentation are maintained in Git.

\- Build directories, generated files, and local database files are excluded using `.gitignore`.

\- Changes are tracked through meaningful commits.

\- Separate branches are used for development activities.

\- Pull Requests are used to integrate completed work.

\- GitHub Actions provides automated build and test verification.

\- Tags identify stable project milestones and releases.



\## 8. Conclusion



The configuration management strategy provides a controlled approach to managing the Moderated LMS project. Branching, meaningful commits, Pull Requests, automated CI checks, and release tags help maintain project quality and traceability throughout development.s

