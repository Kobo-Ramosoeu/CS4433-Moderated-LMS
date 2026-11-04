# Moderated LMS — Role-Based User Guide

## 1. Purpose
This guide summarises the documented workflows for the four application roles. Menu wording may vary by build. Follow the prompts shown by the running application.

## 2. Administrator
The Administrator manages the system's foundational records:
- Create accounts for Students, Lecturers, HoDs and Administrators.
- List, update and activate/deactivate user accounts.
- Create, list and rename departments.
- Assign an active HoD to a department.
- Create, list and update courses.
- Assign Lecturers to courses.
- Enrol Students in courses.

Create the academic structure and user accounts before expecting Lecturer and Student workflows to have data to operate on.

## 3. Lecturer
The Lecturer:
- Creates and views assignments for assigned courses.
- Reviews unmarked submissions.
- Enters marks and mandatory written feedback.
- Grants an individual extension or a whole-class extension with a reason. In the documented workflow, student ID `0` denotes a whole-class extension.
- Reviews and corrects marks returned by the HoD.

When granting an extension, check the assignment/course scope and revised deadline carefully.

## 4. Student
The Student:
- Views enrolled courses and assignments.
- Submits work using a local file path.
- Resubmits where the effective deadline permits.
- Views their own marks and feedback only after HoD release.

The application applies the effective deadline and late-submission rules. The project documentation records a 10% of maximum-mark penalty per completed 24-hour period, up to 72 hours. The implemented detailed use-case rule rejects new first submissions more than 72 hours late.

## 5. Head of Department
The HoD:
- Reviews marks awaiting moderation for the department.
- Approves marks or returns them to the Lecturer with a mandatory reason.
- May adjust a mark during moderation with a mandatory reason.
- Releases approved marks to Students.

Students should not see marks before release.

## 6. Good practice
- Confirm the selected role and course before changing records.
- Provide meaningful reasons for extensions, corrections and moderation adjustments.
- Verify assignment deadlines before submission.
- Keep local assignment files in a known location.
- Report unexpected behaviour to the system administrator.

## 7. Known policy clarification
The specification contains two different descriptions for submissions more than 72 hours late: zero mark versus rejection. The current implementation follows rejection for a new first submission beyond 72 hours. Obtain lecturer confirmation if the policy must be changed.
