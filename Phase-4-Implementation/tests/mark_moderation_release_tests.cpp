#include "lms/Database.h"
#include "lms/MarkingRepository.h"
#include "lms/ModerationRepository.h"
#include <cassert>
#include <cstdio>
#include <iostream>

int main() {
    const char* dbPath="mark_moderation_test.db"; std::remove(dbPath);
    try {
      {
        lms::Database db(dbPath); db.executeFile("database/schema.sql");
        db.execute("INSERT INTO \"User\"(username,password_hash,full_name,role) VALUES"
                   "('student','x','Student','Student'),('lecturer','x','Lecturer','Lecturer'),('hod','x','HoD','HoD');");
        db.execute("INSERT INTO Student(user_id,student_number) VALUES(1,'S001');");
        db.execute("INSERT INTO Lecturer(user_id,staff_number) VALUES(2,'L001');");
        db.execute("INSERT INTO HeadOfDepartment(user_id) VALUES(3);");
        db.execute("INSERT INTO Department(name,hod_user_id) VALUES('Computing',3);");
        db.execute("INSERT INTO Course(department_id,course_code,course_name) VALUES(1,'CS4433','Moderated LMS');");
        db.execute("INSERT INTO CourseLecturer(course_id,lecturer_id) VALUES(1,2);");
        db.execute("INSERT INTO Enrolment(student_id,course_id) VALUES(1,1);");
        db.execute("INSERT INTO Assignment(course_id,title,description,deadline,max_mark) VALUES(1,'Test','', '2099-11-07 23:59:00',100);");
        db.execute("INSERT INTO Submission(assignment_id,student_id,attempt_number,file_path,submitted_at) VALUES(1,1,1,'submission.txt','2026-10-31 12:00:00');");

        lms::MarkingRepository marking(db); lms::ModerationRepository moderation(db);
        const auto initial=marking.getReleasedMarksForStudent(1); assert(initial.empty());
        const auto result=marking.enterMark(2,1,85.0,"Good work"); assert(result.finalMark==85.0);
        assert(marking.getReleasedMarksForStudent(1).empty());
        moderation.moderateMark(3,1,"CorrectionRequired","Please review calculation");
        auto returned=marking.getReturnedMarksForLecturer(2); assert(returned.size()==1);
        const auto corrected=marking.correctReturnedMark(2,1,90.0,"Calculation corrected"); assert(corrected.finalMark==90.0);
        moderation.moderateMark(3,1,"Approved","Correction accepted");
        assert(marking.getReleasedMarksForStudent(1).empty());
        const int released=moderation.releaseApprovedMarks(3); assert(released==1);
        auto visible=marking.getReleasedMarksForStudent(1);
        assert(visible.size()==1 && visible[0].rawMark==90.0 && visible[0].finalMark==90.0);
        // Audit history includes marking, correction, moderation, and release.
        sqlite3_stmt* s=nullptr; sqlite3_prepare_v2(db.getConnection(),"SELECT COUNT(*) FROM AuditRecord WHERE mark_id=1",-1,&s,nullptr);
        sqlite3_step(s); int audits=sqlite3_column_int(s,0); sqlite3_finalize(s); assert(audits>=4);
      }
      std::remove(dbPath); std::cout << "Marking/moderation/release workflow tests passed.\n"; return 0;
    } catch(const std::exception& e) { std::remove(dbPath); std::cerr<<e.what()<<'\n'; return 1; }
}
