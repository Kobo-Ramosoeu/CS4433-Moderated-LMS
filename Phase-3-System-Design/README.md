# Phase 3: System Design

## 1. Overview

Phase 3 focuses on the system design of the **Moderated Learning Management System (LMS)**. It translates the requirements identified in Phase 2 into a technical design that can guide the implementation of the system.

The design defines the system architecture, software components, classes and their relationships, system behaviour, database structure, and security considerations.

The proposed system is a C++17 console-based application using an SQLite database. It is intended to operate offline or on a local network and support assignment management, student submissions, marking, moderation, and controlled release of marks.

## 2. Objectives

The main objectives of Phase 3 are to:

- Define the system's architecture and explain the choice of architecture.
- Identify the major software components and their dependencies.
- Develop a design-level UML class diagram.
- Apply appropriate design patterns and SOLID principles.
- Model the submission, marking, moderation, and mark-release workflows.
- Design a normalized relational database.
- Define the relationships between the software classes and database tables.
- Identify security, data-integrity, and authorization requirements that must be considered during implementation.

## 3. System Scope

The system supports four primary user roles:

| Role | Main responsibilities |
|---|---|
| Administrator | Manage user accounts, departments, courses, lecturer assignments, and student enrolments. |
| Student | View enrolled courses and assignments, submit work, resubmit work where permitted, and view released marks and feedback. |
| Lecturer | Create assignments, set deadlines and maximum marks, grant deadline extensions, mark submissions, and provide feedback. |
| Head of Department (HoD) | Review and moderate marks, request corrections, approve marks, and authorize their release. |

The system does not include online payments, video conferencing, automated grading, plagiarism detection, or web and mobile applications.

## 4. System Architecture

The system uses a layered architecture to separate user interaction, business rules, data access, and persistent storage.

The architecture consists of four layers:

1. **Presentation Layer:** Provides the console menus, accepts user input, and displays results and error messages.
2. **Business Logic Layer:** Handles authentication, authorization, assignment management, submission validation, late penalties, marking, moderation, mark release, and auditing.
3. **Data Access Layer:** Provides repositories or Data Access Objects (DAOs) that handle database queries and persistence.
4. **Database Layer:** Uses SQLite to store user details, courses, assignments, submissions, marks, moderation records, deadline extensions, and audit records.

### Architecture rationale

The layered approach separates responsibilities and makes the system easier to understand, test, maintain, and extend. It also keeps SQL operations separate from the business rules.

For example, the submission service determines whether a submission is late and coordinates the penalty calculation. The repository is responsible for saving the submission. This separation prevents database code from becoming mixed with the application's business rules.

The design also allows the database implementation to be changed with fewer modifications to the business logic, provided the data-access interfaces remain consistent.

## 5. Component Diagram

The component diagram identifies the major software components and shows how they depend on one another.

The main components include:

- **Console UI:** Provides the user interface.
- **Authentication and Authorization:** Verifies users and checks role permissions.
- **Assignment Service:** Manages assignments, deadlines, and extensions.
- **Submission Service:** Validates and records submissions and attempts.
- **Late-Penalty Strategy:** Calculates applicable penalties.
- **Marking Service:** Manages marks and lecturer feedback.
- **Moderation and Release Service:** Handles HoD review, approval, correction requests, and release.
- **Audit Service:** Records important actions and mark changes.
- **Repositories:** Handle data access for users, assignments, submissions, marks, departments, and courses.
- **SQLite Database:** Stores the system's persistent data.

The component dependencies are designed so that the presentation layer communicates with the business logic, while repositories provide access to the database.

## 6. Design-Level UML Class Diagram

The class design identifies the main classes, their attributes and operations, and their relationships.

### Main classes

- `User` — abstract base class for shared user information and behaviour.
- `Student` — represents a student.
- `Lecturer` — represents a lecturer.
- `HeadOfDepartment` — represents a department head.
- `Administrator` — represents an administrator.
- `Department` — represents an academic department.
- `Course` — represents a course offered by a department.
- `Enrolment` — associates students with courses.
- `Assignment` — represents an assignment with a deadline and maximum mark.
- `DeadlineExtension` — records individual or whole-class deadline extensions.
- `Submission` — records a student's submission attempt.
- `Mark` — stores the mark, feedback, and current marking or release status.
- `ModerationRecord` — records HoD moderation decisions.
- `AuditRecord` — records important actions and changes for accountability.

### Important relationships

- A department can offer multiple courses.
- Students and courses have a many-to-many relationship through `Enrolment`.
- Lecturers and courses have a many-to-many relationship.
- A course can contain multiple assignments.
- An assignment can have multiple deadline extensions and submissions.
- A student can submit multiple attempts for an assignment, subject to the submission rules.
- A submission can have at most one current `Mark` record.
- A mark can have multiple moderation records and associated audit records.

The design preserves separate submission attempts rather than overwriting previous attempts. This supports resubmission tracking and provides a record of a student's submission history.

## 7. Design Patterns

Two design patterns are included in the design.

### 7.1 Strategy Pattern

The Strategy pattern is used for late-penalty calculation.

The `ILatePenaltyStrategy` interface defines the penalty-calculation operation, while `StandardLatePenaltyStrategy` implements the current policy.

This design makes it possible to change or introduce another penalty policy without rewriting the submission service.

### 7.2 Repository Pattern

The Repository pattern separates data access from business logic.

For example, `ISubmissionRepository` defines operations for saving and retrieving submissions, while `SQLiteSubmissionRepository` implements those operations using SQLite.

The business logic depends on the repository interface rather than directly depending on SQLite queries.

## 8. SOLID Principles

The design applies the SOLID principles as follows:

- **Single Responsibility Principle (SRP):** Each class has a focused responsibility, such as submission processing, marking, moderation, or data access.
- **Open/Closed Principle (OCP):** The late-penalty strategy can be extended or replaced without modifying the submission service's core workflow.
- **Liskov Substitution Principle (LSP):** Role-specific user classes should be usable wherever the base `User` abstraction is expected, without breaking its contract.
- **Interface Segregation Principle (ISP):** Interfaces should contain focused operations rather than forcing classes to implement unrelated methods.
- **Dependency Inversion Principle (DIP):** Business services should depend on abstractions, such as repository and strategy interfaces, rather than concrete database implementations.

These principles are intended to improve maintainability, flexibility, and testability.

## 9. Behavioural Models

Phase 3 includes behavioural models that describe how the system operates during important workflows.

### 9.1 Submission State Machine

The submission and marking workflow follows these states:

1. Submitted
2. Marked
3. Awaiting Moderation
4. Moderated
5. Released

If the HoD requests corrections, the workflow returns to the marking stage. Marks must not be visible to students before they have been approved and released.

Lateness is treated as a submission property or condition, rather than a separate lifecycle state.

### 9.2 Activity Diagram with Swimlanes

The activity diagram models the workflow from assignment submission through marking, moderation, and release of marks.

It identifies the responsibilities of the Student, Console UI, Submission Service, Repository/SQLite, Lecturer, HoD, and student result-viewing process.

The workflow includes validation, deadline checking, penalty calculation, saving the submission, marking, moderation, correction requests, and final release.

### 9.3 Sequence Diagram: Submit Assignment

This diagram shows the interactions between the student, console interface, submission service, repository, and late-penalty strategy.

The workflow includes:

- Receiving a submission request.
- Validating the user, assignment, and submission data.
- Determining the applicable deadline, including approved extensions.
- Checking previous attempts and resubmission eligibility.
- Calculating the late penalty when necessary.
- Saving a new submission attempt.
- Returning a confirmation or validation error.

### 9.4 Sequence Diagram: Moderate and Release Marks

This diagram models the lecturer's marking process and the HoD's moderation and release process.

The lecturer submits a mark and feedback for moderation. The HoD reviews the mark and either requests corrections or approves it. Approved marks can then be released to students.

Moderation decisions and relevant mark changes must be recorded for accountability.

## 10. Late-Submission Policy

The system applies the agreed late-submission policy:

- Submissions within the applicable deadline do not receive a late penalty.
- Late submissions lose 10% of the assignment's maximum mark for each completed 24-hour period, up to 72 hours.
- The Phase 2 submission use-case exception says new submissions more than three days late are rejected, while Phase 2 BR2 and the original Phase 3 wording say the mark becomes zero. The implementation follows the more specific submission exception for new submissions; this conflict is recorded for formal requirements reconciliation.
- Deadlines are exact local date/time values stored as `YYYY-MM-DD HH:MM:SS`; legacy date-only deadlines are interpreted as 23:59:59.
- An approved deadline extension changes the applicable deadline.
- Extensions may apply to an individual student or the entire class.
- Whole-class and individual extensions must be visible where students view their assignments.

The penalty calculation is based on the assignment's maximum mark, not the student's raw mark. Completed late days are measured as elapsed 24-hour periods, not rounded calendar dates.

For example, if an assignment is worth 100 marks, the penalty is 10 marks per late day, up to three days. The implementation must define and consistently apply how elapsed days are counted and how the final mark is bounded.

## 11. Database Design

The database design uses SQLite and a relational structure intended to satisfy Third Normal Form (3NF).

### Main tables

| Table | Purpose |
|---|---|
| `User` | Stores shared user information and password hashes. |
| `Department` | Stores academic departments and their HoD assignment. |
| `Student` | Stores student-specific details. |
| `Lecturer` | Stores lecturer-specific details. |
| `HeadOfDepartment` | Identifies users who serve as HoDs. |
| `Course` | Stores course information. |
| `CourseLecturer` | Associates lecturers with courses. |
| `Enrolment` | Associates students with courses. |
| `Assignment` | Stores assignment descriptions, deadlines, and maximum marks. |
| `DeadlineExtension` | Stores approved individual and whole-class extensions. |
| `Submission` | Stores submission attempts and their metadata. |
| `Mark` | Stores the current mark, feedback, and status for a submission. |
| `ModerationRecord` | Stores moderation decisions and reasons. |
| `AuditRecord` | Stores important actions and changes. |

### Normalization

The design separates entities into appropriate tables to reduce duplication and update anomalies.

Many-to-many relationships are represented through linking tables, such as `Enrolment` and `CourseLecturer`. Student and lecturer details are separated from shared user information. Submission attempts are stored as separate records, preserving the history of attempts.

The final SQL schema should define primary keys, foreign keys, unique constraints, appropriate `NOT NULL` constraints, and relevant `CHECK` constraints.

### Data dictionary and class-to-table mapping

The data dictionary defines the purpose, data types, and constraints of database fields.

The class-to-table mapping explains how persistent domain objects are represented in the relational database. For example, `Submission` maps to `Submission`, `Assignment` maps to `Assignment`, and `ModerationRecord` maps to `ModerationRecord`.

Service, strategy, and repository classes primarily implement behaviour and data access; they do not necessarily require corresponding database tables.

## 12. Security and Data Integrity

The implementation must observe the following safeguards:

- Store password hashes rather than plaintext passwords, using a recognized password-hashing library with salts.
- Use parameterized SQL statements to reduce SQL injection risk.
- Enforce role-based authorization for administrative, lecturer, student, and HoD operations.
- Validate marks against the assignment's maximum mark.
- Validate submission eligibility and the applicable deadline before saving a submission.
- Prevent students from viewing marks before authorized release.
- Ensure that only authorized lecturers can mark submissions and that only authorized HoDs can moderate and release marks.
- Record important mark changes, including the responsible user, timestamp, previous value, new value, and reason where applicable.
- Apply database constraints and foreign keys to protect data integrity.

Application-level checks and, where appropriate, database constraints or triggers should enforce rules that cannot be expressed through simple column constraints alone.

## 13. Implementation Technology

The proposed implementation environment is:

- **Programming language:** C++17
- **Application interface:** Console-based interface
- **Database:** SQLite
- **Operating mode:** Offline or local network
- **Persistence approach:** Repository/DAO interfaces backed by SQLite

The design targets a system serving approximately 800 students. Implementation and testing should verify that the chosen data-access approach and queries perform adequately for the expected workload.

## 14. Design Assumptions

The design uses the following assumptions:

1. Each department has one designated HoD, and each HoD oversees one department.
2. Students can be enrolled in multiple courses.
3. Courses can have multiple lecturers.
4. Each submission attempt is stored separately.
5. A submission has at most one current mark record; moderation history is maintained separately.
6. A deadline extension may apply to one student or to the whole class.
7. Marks remain unavailable to students until authorized release.
8. Business rules, including penalties and permissions, are validated by the application rather than relying solely on the user interface.

These assumptions should be confirmed against the approved requirements and reflected consistently in the final diagrams and database schema.

## 15. Phase 3 Deliverables

The Phase 3 submission should contain the following artifacts:

- System architecture diagram and rationale.
- UML component diagram.
- Design-level UML class diagram.
- Explanation of the selected design patterns and SOLID principles.
- Submission State Machine diagram.
- Activity Diagram with swimlanes.
- Sequence Diagram for submitting an assignment.
- Sequence Diagram for moderating and releasing marks.
- Database ERD normalized to 3NF.
- Data dictionary.
- SQLite SQL schema defining tables, keys, and constraints.
- Class-to-table mapping and justification.

Each artifact should be consistent with the others. For example, the class diagram, ERD, SQL schema, and sequence diagrams must agree on submission attempts, marks, moderation decisions, deadline extensions, and mark-release rules.

## 16. Conclusion

Phase 3 establishes the technical design for the Moderated LMS before implementation begins. The layered architecture separates responsibilities, the class and component designs organize the software, the behavioural models describe important workflows, and the relational database design provides persistent storage.

The design also addresses the project's central control requirement: lecturers mark submissions, HoDs moderate marks, and students see marks only after authorized release.

This design serves as the basis for the subsequent implementation and testing phases.
