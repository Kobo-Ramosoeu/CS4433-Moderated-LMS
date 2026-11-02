
#include "lms/Database.h"
#include "lms/PasswordHasher.h"
#include "lms/UserRepository.h"
#include "lms/ModerationRepository.h"

#include "lms/models/Student.h"
#include "lms/models/Lecturer.h"
#include "lms/models/HeadOfDepartment.h"
#include "lms/models/Administrator.h"
#include "lms/SubmissionRepository.h"
#include "lms/MarkingRepository.h"
#include "lms/AdministrationRepository.h"
#include "lms/DeadlineExtensionRepository.h"
#include <sqlite3.h>

#include <exception>
#include <iostream>
#include <memory>
#include <string>

#include "lms/AssignmentRepository.h"
#include <limits>


int showDashboard(const lms::User& user)
{
    int choice;

    std::cout << "\n================================\n";
    std::cout << "       MODERATED LMS\n";
    std::cout << "================================\n";
    std::cout << "Welcome, " << user.getFullName() << "!\n";
    std::cout << "Username: " << user.getUsername() << '\n';
    std::cout << "Role: " << user.getRole() << '\n';

    if (user.getRole() == "Student")
    {
        std::cout << "\n=== Student Dashboard ===\n";
        std::cout << "1. View enrolled courses\n";
        std::cout << "2. View assignments\n";
        std::cout << "3. Submit / resubmit assignment\n";
        std::cout << "4. View released marks\n";
        std::cout << "0. Logout\n";
    }
    else if (user.getRole() == "Lecturer")
    {
        std::cout << "\n=== Lecturer Dashboard ===\n";
        std::cout << "1. Manage assignments\n";
        std::cout << "2. Mark submissions\n";
        std::cout << "3. Grant deadline extension (individual or whole class)\n";
        std::cout << "4. Review marks returned for correction\n";
        std::cout << "0. Logout\n";
    }
    else if (user.getRole() == "HoD")
    {
        std::cout << "\n=== Head of Department Dashboard ===\n";
        std::cout << "1. Review marks for moderation\n";
        std::cout << "2. Approve marks\n";
        std::cout << "3. Return marks for correction\n";
        std::cout << "4. Adjust a mark (reason required)\n";
        std::cout << "5. Release all approved marks\n";
        std::cout << "0. Logout\n";
    }
    else if (user.getRole() == "Administrator")
    {
        std::cout << "\n=== Administrator Dashboard ===\n";
        std::cout << "1. Open administration workspace\n";
        std::cout << "0. Logout\n";
    }

    std::cout << "Choose an option: ";

    if (!(std::cin >> choice))
    {
        std::cin.clear();
        std::cin.ignore(
            std::numeric_limits<std::streamsize>::max(), '\n');

        return -1;
    }

    std::cin.ignore(
        std::numeric_limits<std::streamsize>::max(), '\n');

    return choice;
}



void manageAssignments(lms::AssignmentRepository& repository,
                       int lecturerId)
{
    int choice;

    while (true)
    {
        std::cout << "\n=== Assignment Management ===\n";
        std::cout << "1. Create assignment\n";
        std::cout << "2. View assignments\n";
        std::cout << "0. Return to dashboard\n";
        std::cout << "Choose an option: ";

        if (!(std::cin >> choice))
        {
            std::cin.clear();
            std::cin.ignore(
                std::numeric_limits<std::streamsize>::max(), '\n');

            std::cout << "Invalid input. Enter a number.\n";
            continue;
        }

        std::cin.ignore(
            std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice == 0)
        {
            return;
        }

        try
        {
            if (choice == 1)
            {
                auto courses =
                    repository.getCoursesForLecturer(lecturerId);

                if (courses.empty())
                {
                    std::cout
                        << "You have no courses assigned to you.\n";
                    continue;
                }

                std::cout << "\n=== Your Courses ===\n";

                for (const auto& course : courses)
                {
                    std::cout << course.courseId << ". "
                              << course.courseCode << " - "
                              << course.courseName << '\n';
                }

                int courseId;

                std::cout << "Enter course ID: ";

                if (!(std::cin >> courseId))
                {
                    std::cin.clear();
                    std::cin.ignore(
                        std::numeric_limits<std::streamsize>::max(),
                        '\n');

                    std::cout << "Invalid course ID.\n";
                    continue;
                }

                std::cin.ignore(
                    std::numeric_limits<std::streamsize>::max(), '\n');

                std::string title;
                std::string description;
                std::string deadline;
                double maxMark;

                std::cout << "Assignment title: ";
                std::getline(std::cin, title);

                std::cout << "Description: ";
                std::getline(std::cin, description);

                std::cout
                    << "Deadline (YYYY-MM-DD HH:MM): ";
                std::getline(std::cin, deadline);

                std::cout << "Maximum mark: ";

                if (!(std::cin >> maxMark))
                {
                    std::cin.clear();
                    std::cin.ignore(
                        std::numeric_limits<std::streamsize>::max(),
                        '\n');

                    std::cout << "Invalid maximum mark.\n";
                    continue;
                }

                std::cin.ignore(
                    std::numeric_limits<std::streamsize>::max(), '\n');

                repository.createAssignment(
                    lecturerId,
                    courseId,
                    title,
                    description,
                    deadline,
                    maxMark);

                std::cout
                    << "\nAssignment created successfully!\n";
            }
            else if (choice == 2)
            {
                auto assignments =
                    repository.getAssignmentsForLecturer(lecturerId);

                if (assignments.empty())
                {
                    std::cout << "No assignments found.\n";
                    continue;
                }

                std::cout << "\n=== Your Assignments ===\n";

                for (const auto& assignment : assignments)
                {
                    std::cout << "\nAssignment ID: "
                              << assignment.getAssignmentId()
                              << "\nCourse ID: "
                              << assignment.getCourseId()
                              << "\nTitle: "
                              << assignment.getTitle()
                              << "\nDescription: "
                              << assignment.getDescription()
                              << "\nDeadline: "
                              << assignment.getDeadline()
                              << "\nMaximum mark: "
                              << assignment.getMaxMark()
                              << "\n";
                }
            }
            else
            {
                std::cout << "Invalid option.\n";
            }
        }
        catch (const std::exception& error)
        {
            std::cout << "Error: " << error.what() << '\n';
        }
    }
}


void viewStudentAssignments(
    lms::AssignmentRepository& repository,
    lms::Database& database,
    int studentId)
{
    try
    {
        const auto assignments = repository.getAssignmentsForStudent(studentId);
        std::cout << "\n=== Assignments for Your Enrolled Courses ===\n";
        if (assignments.empty()) { std::cout << "No assignments found for your enrolled courses.\n"; return; }
        lms::DeadlineExtensionRepository extensions(database);
        for (const auto& a : assignments)
        {
            std::cout << "\nAssignment ID: " << a.getAssignmentId()
                      << "\nCourse ID: " << a.getCourseId()
                      << "\nTitle: " << a.getTitle()
                      << "\nDescription: " << a.getDescription()
                      << "\nOriginal deadline: " << a.getDeadline()
                      << "\nYour effective deadline: " << extensions.getEffectiveDeadline(a.getAssignmentId(), studentId)
                      << "\nMaximum mark: " << a.getMaxMark() << "\n";
        }
    }
    catch (const std::exception& e) { std::cout << "Error: " << e.what() << '\n'; }
}

void submitStudentAssignment(
    lms::AssignmentRepository& assignmentRepository,
    lms::SubmissionRepository& submissionRepository,
    lms::Database& database,
    int studentId)
{
    auto assignments =
        assignmentRepository.getAssignmentsForStudent(studentId);

    if (assignments.empty())
    {
        std::cout << "\nYou have no assignments available.\n";
        return;
    }

    std::cout << "\n===== Submit / Resubmit Assignment =====\n";
    lms::DeadlineExtensionRepository extensions(database);
    for (const auto& assignment : assignments)
    {
        std::cout << "ID: " << assignment.getAssignmentId()
                  << " | " << assignment.getTitle()
                  << " | Effective deadline: "
                  << extensions.getEffectiveDeadline(assignment.getAssignmentId(), studentId)
                  << '\n';
    }

    int assignmentId;

    std::cout << "\nEnter assignment ID: ";
    std::cin >> assignmentId;
    std::cin.ignore(
        std::numeric_limits<std::streamsize>::max(), '\n');

    std::string filePath;

    std::cout << "Enter the path to your assignment file: ";
    std::getline(std::cin, filePath);

    try
    {
        submissionRepository.submitAssignment(studentId, assignmentId, filePath);
    }
    catch (const std::exception& error)
    {
        std::cout << "Submission failed: "
                  << error.what() << '\n';
    }
}


void viewReleasedMarks(lms::MarkingRepository& repository, int studentId)
{
    try
    {
        const auto marks = repository.getReleasedMarksForStudent(studentId);
        std::cout << "\n=== Your Released Marks ===\n";
        if (marks.empty())
        {
            std::cout << "No marks have been released to you yet.\n";
            return;
        }
        for (const auto& mark : marks)
        {
            std::cout << "\nCourse: " << mark.courseCode
                      << "\nAssignment: " << mark.assignmentTitle
                      << "\nRaw mark: " << mark.rawMark << "/" << mark.maxMark
                      << "\nFinal mark: " << mark.finalMark << "/" << mark.maxMark
                      << "\nFeedback: " << mark.feedback
                      << "\nMarked at: " << mark.markedAt << "\n";
        }
    }
    catch (const std::exception& error)
    {
        std::cout << "Could not retrieve released marks: " << error.what() << '\n';
    }
}

void reviewSubmissions(lms::MarkingRepository& repository,
                       int lecturerId)
{
    try
    {
        auto submissions =
            repository.getSubmissionsForLecturer(lecturerId);

        if (submissions.empty())
        {
            std::cout << "\nNo unmarked submissions found.\n";
            return;
        }

        std::cout << "\n=== Submissions Awaiting Marks ===\n";

        for (const auto& submission : submissions)
        {
            std::cout << "\nSubmission ID: "
                      << submission.submissionId
                      << "\nAssignment: "
                      << submission.assignmentTitle
                      << "\nStudent number: "
                      << submission.studentNumber
                      << "\nAttempt: "
                      << submission.attemptNumber
                      << "\nSubmitted at: "
                      << submission.submittedAt
                      << "\nMaximum mark: "
                      << submission.maxMark << '\n';
        }

        int submissionId;
        std::cout << "\nEnter submission ID to mark (0 to cancel): ";

        if (!(std::cin >> submissionId))
        {
            std::cin.clear();
            std::cin.ignore(
                std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input.\n";
            return;
        }

        if (submissionId == 0)
        {
            return;
        }

        double rawMark;
        std::cout << "Enter raw mark: ";

        if (!(std::cin >> rawMark))
        {
            std::cin.clear();
            std::cin.ignore(
                std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid mark.\n";
            return;
        }

        std::cin.ignore(
            std::numeric_limits<std::streamsize>::max(), '\n');

        std::string feedback;
        std::cout << "Enter feedback: ";
        std::getline(std::cin, feedback);

        const auto result = repository.enterMark(
            lecturerId, submissionId, rawMark, feedback);

        std::cout << "\nMark saved successfully!\n"
                  << "Days late: " << result.daysLate
                  << "\nRaw mark: " << result.rawMark
                  << "\nPenalty: " << result.penalty
                  << "\nFinal mark: " << result.finalMark
                  << "\nStatus: AwaitingModeration\n";
    }
    catch (const std::exception& error)
    {
        std::cout << "\nCould not mark submission: "
                  << error.what() << '\n';
    }
}



void reviewReturnedMarks(lms::MarkingRepository& repository, int lecturerId)
{
    try
    {
        const auto marks = repository.getReturnedMarksForLecturer(lecturerId);
        if (marks.empty())
        {
            std::cout << "\nNo marks have been returned for correction.\n";
            return;
        }
        std::cout << "\n=== Marks Returned for Correction ===\n";
        for (const auto& mark : marks)
        {
            std::cout << "\nMark ID: " << mark.markId
                      << "\nSubmission ID: " << mark.submissionId
                      << "\nAssignment: " << mark.assignmentTitle
                      << "\nStudent: " << mark.studentNumber
                      << "\nRaw mark: " << mark.rawMark << "/" << mark.maxMark
                      << "\nFeedback: " << mark.feedback
                      << "\nHoD correction reason: " << mark.correctionReason << '\n';
        }
        int markId;
        std::cout << "\nEnter Mark ID to correct (0 to cancel): ";
        if (!(std::cin >> markId))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input.\n";
            return;
        }
        if (markId == 0) return;
        double rawMark;
        std::cout << "Enter corrected raw mark: ";
        if (!(std::cin >> rawMark))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid mark.\n";
            return;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::string feedback;
        std::cout << "Enter updated feedback: ";
        std::getline(std::cin, feedback);
        const auto result = repository.correctReturnedMark(
            lecturerId, markId, rawMark, feedback);
        std::cout << "\nCorrected mark saved and sent back for moderation.\n"
                  << "Days late: " << result.daysLate
                  << "\nRaw mark: " << result.rawMark
                  << "\nPenalty: " << result.penalty
                  << "\nFinal mark: " << result.finalMark
                  << "\nStatus: AwaitingModeration\n";
    }
    catch (const std::exception& error)
    {
        std::cout << "\nCould not correct mark: " << error.what() << '\n';
    }
}


void reviewMarksForModeration(
    lms::Database& database,
    int hodId)
{
    lms::ModerationRepository repository(database);

    try
    {
        auto marks = repository.getMarksAwaitingModeration(hodId);

        if (marks.empty())
        {
            std::cout << "\nNo marks are awaiting moderation "
                         "in your department.\n";
            return;
        }

        std::cout << "\n=== Marks Awaiting Moderation ===\n";

        for (const auto& mark : marks)
        {
            std::cout << "\nMark ID: " << mark.markId
                      << "\nAssignment: " << mark.assignmentTitle
                      << "\nStudent: " << mark.studentNumber
                      << "\nMarked by: " << mark.lecturerName
                      << "\nRaw mark: " << mark.rawMark
                      << "\nFinal mark: " << mark.finalMark
                      << "\nFeedback: " << mark.feedback
                      << "\nStatus: " << mark.status
                      << "\n";
        }
    }
    catch (const std::exception& error)
    {
        std::cerr << "Moderation error: " << error.what() << '\n';
    }
}

void moderateMark(
    lms::Database& database,
    int hodId,
    const std::string& decision)
{
    int markId;

    std::cout << "Enter Mark ID: ";
    if (!(std::cin >> markId))
    {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "Invalid Mark ID.\n";
        return;
    }

    std::cin.ignore(10000, '\n');

    std::string reason;

    if (decision == "CorrectionRequired")
    {
        std::cout << "Enter reason for correction: ";
        std::getline(std::cin, reason);
    }
    else
    {
        std::cout << "Optional approval comment (press Enter to skip): ";
        std::getline(std::cin, reason);
    }

    lms::ModerationRepository repository(database);

    try
    {
        repository.moderateMark(hodId, markId, decision, reason);

        if (decision == "Approved")
        {
            std::cout << "Mark approved successfully.\n";
        }
        else
        {
            std::cout << "Mark returned for correction successfully.\n";
        }
    }
    catch (const std::exception& error)
    {
        std::cerr << "Moderation failed: " << error.what() << '\n';
    }
}


void releaseApprovedMarks(lms::Database& database, int hodId)
{
    lms::ModerationRepository repository(database);
    try
    {
        const auto marks = repository.getApprovedMarksForRelease(hodId);
        if (marks.empty())
        {
            std::cout << "\nThere are no approved marks waiting for release.\n";
            return;
        }
        std::cout << "\n=== Approved Marks Ready for Release ===\n";
        for (const auto& mark : marks)
            std::cout << "Mark ID: " << mark.markId << " | " << mark.assignmentTitle
                      << " | Student: " << mark.studentNumber
                      << " | Final: " << mark.finalMark << " | " << mark.feedback << '\n';
        std::cout << "\nRelease ALL " << marks.size() << " approved marks? (y/n): ";
        std::string confirmation;
        std::getline(std::cin, confirmation);
        if (confirmation != "y" && confirmation != "Y")
        {
            std::cout << "Release cancelled. No marks were changed.\n";
            return;
        }
        const int released = repository.releaseApprovedMarks(hodId);
        std::cout << released << " mark(s) released successfully. Students can now view them.\n";
    }
    catch (const std::exception& error)
    {
        std::cout << "Could not release marks: " << error.what() << '\n';
    }
}




void adjustModeratedMark(lms::Database& db,int hodId)
{
    int markId; double mark; std::string reason;
    std::cout<<"Mark ID to adjust: ";
    if(!(std::cin>>markId)){std::cin.clear();std::cin.ignore(10000,'\n');std::cout<<"Invalid ID.\n";return;}
    std::cout<<"New raw mark: ";
    if(!(std::cin>>mark)){std::cin.clear();std::cin.ignore(10000,'\n');std::cout<<"Invalid mark.\n";return;}
    std::cin.ignore(10000,'\n');
    std::cout<<"Mandatory reason for adjustment: ";std::getline(std::cin,reason);
    try { lms::ModerationRepository repo(db);repo.adjustMark(hodId,markId,mark,reason);std::cout<<"Mark adjusted and audit record saved. It remains awaiting moderation.\n"; }
    catch(const std::exception& e){std::cout<<"Could not adjust mark: "<<e.what()<<'\n';}
}

void administratorWorkflow(lms::Database& db, int currentAdminId)
{
    lms::AdministrationRepository repo(db);
    while (true)
    {
        std::cout << "\n=== Administrator Management ===\n"
                  << "1. Manage user accounts\n2. Manage departments\n"
                  << "3. Manage courses\n4. Assign lecturers to courses\n"
                  << "5. Enrol students in courses\n6. Assign Heads of Department\n"
                  << "7. View system information\n0. Return\nChoice: ";
        int c;
        if (!(std::cin >> c)) { std::cin.clear(); std::cin.ignore(10000, '\n'); continue; }
        std::cin.ignore(10000, '\n');
        if (c == 0) return;
        try
        {
            if (c == 1)
            {
                std::cout << "1. Create account  2. List accounts  3. Update account  4. Activate/deactivate account\nChoice: ";
                int x; if (!(std::cin >> x)) { std::cin.clear(); std::cin.ignore(10000, '\n'); continue; }
                std::cin.ignore(10000, '\n');
                if (x == 1)
                {
                    std::string username, name, password, role, identifier;
                    std::cout << "Username: "; std::getline(std::cin, username);
                    std::cout << "Full name: "; std::getline(std::cin, name);
                    std::cout << "Password: "; std::getline(std::cin, password);
                    std::cout << "Role (Student, Lecturer, HoD, Administrator): "; std::getline(std::cin, role);
                    if (role == "Student") { std::cout << "Student number: "; std::getline(std::cin, identifier); }
                    else if (role == "Lecturer") { std::cout << "Staff number: "; std::getline(std::cin, identifier); }
                    repo.createAccount(username, lms::PasswordHasher::hashPassword(password), name, role, identifier);
                    std::cout << "Account created with its role profile.\n";
                }
                else if (x == 2)
                {
                    for (const auto& u : repo.getUsers())
                        std::cout << u.id << " | " << u.username << " | " << u.fullName << " | " << u.role << " | " << (u.active ? "Active" : "Inactive") << '\n';
                }
                else if (x == 3)
                {
                    int id; std::string username, name;
                    std::cout << "User ID: "; std::cin >> id; std::cin.ignore(10000, '\n');
                    std::cout << "New username: "; std::getline(std::cin, username);
                    std::cout << "New full name: "; std::getline(std::cin, name);
                    repo.updateAccount(id, username, name);
                    std::cout << "Account updated.\n";
                }
                else if (x == 4)
                {
                    int id, active;
                    std::cout << "User ID: "; std::cin >> id;
                    std::cout << "1 Activate / 0 Deactivate: "; std::cin >> active; std::cin.ignore(10000, '\n');
                    repo.setUserActive(id, active == 1);
                    std::cout << "Account status updated.\n";
                }
            }
            else if (c == 2)
            {
                std::cout << "1. Create department  2. List departments  3. Rename department\nChoice: ";
                int x; std::cin >> x; std::cin.ignore(10000, '\n');
                if (x == 1)
                {
                    std::string name; std::cout << "Department name: "; std::getline(std::cin, name);
                    repo.createDepartment(name); std::cout << "Department created.\n";
                }
                else if (x == 2)
                {
                    for (const auto& d : repo.getDepartments())
                        std::cout << d.id << " | " << d.name << " | HoD user ID: " << d.hodId << '\n';
                }
                else if (x == 3)
                {
                    int id; std::string name; std::cout << "Department ID: "; std::cin >> id; std::cin.ignore(10000, '\n');
                    std::cout << "New name: "; std::getline(std::cin, name);
                    repo.updateDepartment(id, name); std::cout << "Department updated.\n";
                }
            }
            else if (c == 3)
            {
                std::cout << "1. Create course  2. List courses  3. Update course\nChoice: ";
                int x; std::cin >> x; std::cin.ignore(10000, '\n');
                if (x == 1)
                {
                    for (const auto& d : repo.getDepartments()) std::cout << d.id << " | " << d.name << '\n';
                    int did; std::string code, name; std::cout << "Department ID: "; std::cin >> did; std::cin.ignore(10000, '\n');
                    std::cout << "Course code: "; std::getline(std::cin, code);
                    std::cout << "Course name: "; std::getline(std::cin, name);
                    repo.createCourse(did, code, name); std::cout << "Course created.\n";
                }
                else if (x == 2)
                {
                    for (const auto& a : repo.getCourses()) std::cout << a.id << " | " << a.code << " | " << a.name << " | " << a.department << '\n';
                }
                else if (x == 3)
                {
                    int id, did; std::string code, name;
                    std::cout << "Course ID: "; std::cin >> id; std::cout << "Department ID: "; std::cin >> did; std::cin.ignore(10000, '\n');
                    std::cout << "New course code: "; std::getline(std::cin, code);
                    std::cout << "New course name: "; std::getline(std::cin, name);
                    repo.updateCourse(id, did, code, name); std::cout << "Course updated.\n";
                }
            }
            else if (c == 4)
            {
                for (const auto& a : repo.getCourses()) std::cout << a.id << " | " << a.code << " | " << a.name << '\n';
                int course, lecturer; std::cout << "Course ID: "; std::cin >> course;
                std::cout << "Lecturer user ID: "; std::cin >> lecturer; std::cin.ignore(10000, '\n');
                repo.assignLecturer(course, lecturer); std::cout << "Lecturer assigned to course.\n";
            }
            else if (c == 5)
            {
                for (const auto& a : repo.getCourses()) std::cout << a.id << " | " << a.code << " | " << a.name << '\n';
                int course, student; std::cout << "Course ID: "; std::cin >> course;
                std::cout << "Student user ID: "; std::cin >> student; std::cin.ignore(10000, '\n');
                repo.enrolStudent(course, student); std::cout << "Student enrolled.\n";
            }
            else if (c == 6)
            {
                for (const auto& d : repo.getDepartments()) std::cout << d.id << " | " << d.name << '\n';
                int department, hod; std::cout << "Department ID: "; std::cin >> department;
                std::cout << "HoD user ID: "; std::cin >> hod; std::cin.ignore(10000, '\n');
                repo.setHeadOfDepartment(department, hod); std::cout << "HoD assigned to department.\n";
            }
            else if (c == 7)
            {
                std::cout << "Users: " << repo.getUsers().size()
                          << "\nDepartments: " << repo.getDepartments().size()
                          << "\nCourses: " << repo.getCourses().size() << '\n';
            }
            else std::cout << "Invalid option.\n";
        }
        catch (const std::exception& e) { std::cout << "Administrator operation failed: " << e.what() << '\n'; }
    }
}

void viewStudentCourses(lms::AdministrationRepository& repo,int studentId)
{
    try { auto courses=repo.getStudentCourses(studentId); std::cout<<"\n=== Your Enrolled Courses ===\n";
        if(courses.empty()){std::cout<<"You are not enrolled in any courses.\n";return;}
        for(const auto& c:courses)std::cout<<c.code<<" | "<<c.name<<" | "<<c.department<<'\n';
    } catch(const std::exception& e){std::cout<<"Could not retrieve courses: "<<e.what()<<'\n';}
}

void grantDeadlineExtension(lms::Database& db, int lecturerId)
{
    try
    {
        lms::AssignmentRepository assignments(db);

        auto courses = assignments.getCoursesForLecturer(lecturerId);

        if (courses.empty())
        {
            std::cout << "You have no assigned courses.\n";
            return;
        }

        auto all = assignments.getAssignmentsForLecturer(lecturerId);

        if (all.empty())
        {
            std::cout << "No assignments available.\n";
            return;
        }

        std::cout << "\n=== Your Assignments ===\n";

        for (const auto& a : all)
        {
            std::cout << a.getAssignmentId()
                      << " | " << a.getTitle()
                      << " | Deadline: " << a.getDeadline()
                      << '\n';
        }

        int aid;
        int sid;
        std::string deadline;
        std::string reason;

        std::cout << "\nAssignment ID: ";

        if (!(std::cin >> aid))
        {
            std::cin.clear();
            std::cin.ignore(
                std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid assignment ID.\n";
            return;
        }

        std::cout << "Student user ID (0 = whole class): ";

        if (!(std::cin >> sid))
        {
            std::cin.clear();
            std::cin.ignore(
                std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid student ID.\n";
            return;
        }

        std::cin.ignore(
            std::numeric_limits<std::streamsize>::max(), '\n');

        std::cout << "New deadline (YYYY-MM-DD): ";
        std::getline(std::cin, deadline);

        std::cout << "Reason: ";
        std::getline(std::cin, reason);

        lms::DeadlineExtensionRepository extensions(db);

        extensions.grantExtension(
            lecturerId, aid, sid, deadline, reason);

        std::cout << (sid == 0
            ? "Whole-class extension granted.\n"
            : "Individual extension granted.\n");
    }
    catch (const std::exception& e)
    {
        std::cout << "Could not grant extension: "
                  << e.what() << '\n';
    }
}

int main()
{
    try
    {
        lms::Database database("lms.db");
        database.executeFile("database/schema.sql");

        // Backward-compatible migration for databases created before account deactivation.
        bool hasActive = false;
        sqlite3_stmt* info = nullptr;
        if (sqlite3_prepare_v2(database.getConnection(), "PRAGMA table_info('User');", -1, &info, nullptr) == SQLITE_OK)
        {
            while (sqlite3_step(info) == SQLITE_ROW)
            {
                const auto* n = sqlite3_column_text(info, 1);
                if (n && std::string(reinterpret_cast<const char*>(n)) == "is_active") hasActive = true;
            }
        }
        sqlite3_finalize(info);
        if (!hasActive) database.execute("ALTER TABLE \"User\" ADD COLUMN is_active INTEGER NOT NULL DEFAULT 1;");

        lms::UserRepository users(database);
        lms::AdministrationRepository admin(database);

        // First-run setup: create the initial administrator if none exists.
        sqlite3_stmt* countStmt = nullptr;
        int adminCount = 0;
        sqlite3_prepare_v2(database.getConnection(), "SELECT COUNT(*) FROM \"User\" WHERE role='Administrator' AND is_active=1;", -1, &countStmt, nullptr);
        if (countStmt && sqlite3_step(countStmt) == SQLITE_ROW) adminCount = sqlite3_column_int(countStmt, 0);
        sqlite3_finalize(countStmt);
        if (adminCount == 0)
        {
            std::cout << "\n=== Initial Administrator Setup ===\n";
            std::string username, name, password;
            std::cout << "Create the first administrator account.\nUsername: "; std::getline(std::cin, username);
            std::cout << "Full name: "; std::getline(std::cin, name);
            std::cout << "Password: "; std::getline(std::cin, password);
            admin.createAccount(username, lms::PasswordHasher::hashPassword(password), name, "Administrator", "");
            std::cout << "Administrator account created. Please log in.\n";
        }

        while (true)
        {
            int choice;
            std::cout << "\n=== Moderated LMS ===\n1. Login\n0. Exit\nChoose an option: ";
            if (!(std::cin >> choice))
            {
                std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid choice.\n"; continue;
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (choice == 0) { std::cout << "Goodbye!\n"; break; }
            if (choice != 1) { std::cout << "Invalid choice.\n"; continue; }

            std::string username, password;
            std::cout << "\n=== User Login ===\nUsername: "; std::getline(std::cin, username);
            std::cout << "Password: "; std::getline(std::cin, password);
            lms::LoginDetails details;
            if (!users.findLoginDetails(username, details) ||
                !lms::PasswordHasher::verifyPassword(password, details.passwordHash))
            {
                std::cout << "Invalid username or password.\n"; continue;
            }

            std::unique_ptr<lms::User> user;
            if (details.role == "Student") user = std::make_unique<lms::Student>(details.userId,details.username,details.passwordHash,details.fullName);
            else if (details.role == "Lecturer") user = std::make_unique<lms::Lecturer>(details.userId,details.username,details.passwordHash,details.fullName);
            else if (details.role == "HoD") user = std::make_unique<lms::HeadOfDepartment>(details.userId,details.username,details.passwordHash,details.fullName);
            else if (details.role == "Administrator") user = std::make_unique<lms::Administrator>(details.userId,details.username,details.passwordHash,details.fullName);
            else { std::cout << "Unrecognized role.\n"; continue; }

            std::cout << "\nLogin successful!\n";
            while (true)
            {
                int c = showDashboard(*user);
                if (c == 0) { std::cout << "Logged out successfully.\n"; break; }
                if (c == -1) { std::cout << "Invalid input.\n"; continue; }
                const int uid = user->getUserId();
                const std::string role = user->getRole();

                if (role == "Administrator" && c == 1) administratorWorkflow(database, uid);
                else if (role == "Administrator") std::cout << "Invalid option.\\n";
                else if (role == "Lecturer")
                {
                    if (c == 1) { lms::AssignmentRepository r(database); manageAssignments(r,uid); }
                    else if (c == 2) { lms::MarkingRepository r(database); reviewSubmissions(r,uid); }
                    else if (c == 3) grantDeadlineExtension(database,uid);
                    else if (c == 4) { lms::MarkingRepository r(database); reviewReturnedMarks(r,uid); }
                    else std::cout << "Invalid option.\n";
                }
                else if (role == "Student")
                {
                    if(c==1) { lms::AdministrationRepository r(database); viewStudentCourses(r,uid); }
                    else if(c==2) { lms::AssignmentRepository r(database); viewStudentAssignments(r,database,uid); }
                    else if(c==3) { lms::AssignmentRepository a(database); lms::SubmissionRepository r(database); submitStudentAssignment(a,r,database,uid); }
                    else if(c==4) { lms::MarkingRepository r(database); viewReleasedMarks(r,uid); }
                    else std::cout<<"Invalid option.\n";
                }
                else if(role=="HoD")
                {
                    if(c==1) reviewMarksForModeration(database,uid);
                    else if(c==2) moderateMark(database,uid,"Approved");
                    else if(c==3) moderateMark(database,uid,"CorrectionRequired");
                    else if(c==4) adjustModeratedMark(database,uid);
                    else if(c==5) releaseApprovedMarks(database,uid);
                    else std::cout<<"Invalid option.\n";
                }
            }
        }
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Fatal error: " << error.what() << '\n';
        return 1;
    }
}
