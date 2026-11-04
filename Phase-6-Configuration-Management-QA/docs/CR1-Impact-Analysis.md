# CR1 Impact Analysis — Whole-Class Deadline Extensions

## 1. Change request
CR1 requires a Lecturer to extend an assignment deadline for a whole class, not only an individual student, and requires the extension to be reflected in the student's assignment list.

## 2. Affected roles
- Lecturer: creates the class-wide extension and supplies the revised deadline and reason.
- Student: sees the effective deadline applicable to their assignment.
- Administrator: maintains the course, lecturer and enrolment records on which the class scope depends.

## 3. Design and implementation impact
The change affects:
- Deadline extension data and its optional student scope
- Deadline resolution logic, which must consider both a student-specific extension and a class-wide extension
- Lecturer workflow for granting an extension
- Student assignment-list display
- Submission validation and late-penalty calculation, which must use the effective deadline
- Auditability and tests for extension scope and precedence

The current project README documents individual and whole-class extensions. It documents entering student ID `0` in the Lecturer workflow to represent a whole-class extension. The extension reason and new deadline are captured.

## 4. Expected acceptance behaviour
1. A Lecturer selects an assignment/course and grants a new deadline to the whole class.
2. The system stores the extension with class-wide scope and the granting Lecturer/reason.
3. Students enrolled in the relevant class see the applicable revised deadline on their assignment list.
4. Submission validation uses the effective deadline for each affected student.
5. Students outside the class are not affected.
6. A student-specific extension remains scoped to that student and must not unintentionally change the rest of the class.
7. The extension action is traceable.

## 5. Verification required
The codebase and project documentation record support for whole-class and individual extensions. The following end-to-end checks must still be executed against a disposable database before claiming full acceptance:
- Grant class-wide extension and inspect assignment list as two enrolled students.
- Inspect assignment list as a student outside the class.
- Submit before and after the revised deadline and verify penalty calculation.
- Grant an individual extension and verify its scope and interaction with the class-wide extension.
- Verify extension reason and granting user are persisted.

## 6. Specification and policy note
The project also contains a late-submission rule ambiguity: one requirement describes a zero mark after three days, while the detailed Submit Assignment use case specifies rejection after 72 hours. The current implementation follows the detailed use-case rule for new first submissions. This discrepancy should be confirmed with the lecturer/product owner; it must not be presented as resolved without that confirmation.

## 7. Conclusion
CR1 is represented in the implementation and documented workflows. Final acceptance depends on executing the class-wide extension scenarios above and confirming that the revised deadline is visible in the student assignment list.
