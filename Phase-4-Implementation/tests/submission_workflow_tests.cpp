#include "lms/Database.h"
#include "lms/SubmissionRepository.h"
#include <cassert>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

static std::string localStamp(std::chrono::system_clock::time_point point) {
    std::time_t t = std::chrono::system_clock::to_time_t(point);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream out; out << std::put_time(&tm, "%Y-%m-%d %H:%M:%S"); return out.str();
}
static int countSubmissions(lms::Database& db) {
    sqlite3_stmt* s=nullptr; sqlite3_prepare_v2(db.getConnection(), "SELECT COUNT(*) FROM Submission", -1, &s, nullptr);
    sqlite3_step(s); int n=sqlite3_column_int(s,0); sqlite3_finalize(s); return n;
}
static int maxAttempt(lms::Database& db) {
    sqlite3_stmt* s=nullptr; sqlite3_prepare_v2(db.getConnection(), "SELECT COALESCE(MAX(attempt_number),0) FROM Submission", -1, &s, nullptr);
    sqlite3_step(s); int n=sqlite3_column_int(s,0); sqlite3_finalize(s); return n;
}
int main() {
    const char* dbPath="submission_workflow_test.db"; const char* filePath="submission_test_file.txt";
    std::remove(dbPath); { std::ofstream f(filePath); f << "test submission"; }
    try {
      {
        lms::Database db(dbPath); db.executeFile("database/schema.sql");
        db.execute("INSERT INTO \"User\"(username,password_hash,full_name,role) VALUES('s','x','Student','Student');");
        db.execute("INSERT INTO Student(user_id,student_number) VALUES(1,'S1');");
        db.execute("INSERT INTO Department(name) VALUES('D');");
        db.execute("INSERT INTO Course(department_id,course_code,course_name) VALUES(1,'C1','Course');");
        db.execute("INSERT INTO Enrolment(student_id,course_id) VALUES(1,1);");
        auto now=std::chrono::system_clock::now();
        db.execute("INSERT INTO Assignment(course_id,title,description,deadline,max_mark) VALUES(1,'Late accepted','', '"+localStamp(now-std::chrono::hours(71))+"',100);");
        lms::SubmissionRepository repo(db);
        repo.submitAssignment(1,1,filePath);
        assert(countSubmissions(db)==1 && maxAttempt(db)==1);
        // First late submission beyond 72 hours must be rejected.
        db.execute("INSERT INTO Assignment(course_id,title,description,deadline,max_mark) VALUES(1,'Too late','', '"+localStamp(now-std::chrono::hours(73))+"',100);");
        bool lateRejected=false; try { repo.submitAssignment(1,2,filePath); } catch(const std::runtime_error&) { lateRejected=true; }
        assert(lateRejected && countSubmissions(db)==1);
        // Multiple submissions before the effective deadline are allowed and numbered.
        db.execute("INSERT INTO Assignment(course_id,title,description,deadline,max_mark) VALUES(1,'Resubmit','', '"+localStamp(now+std::chrono::hours(2))+"',100);");
        repo.submitAssignment(1,3,filePath); repo.submitAssignment(1,3,filePath);
        assert(countSubmissions(db)==3 && maxAttempt(db)==2);
        // Once a first attempt exists, a resubmission after the deadline is rejected.
        db.execute("UPDATE Assignment SET deadline='"+localStamp(now-std::chrono::minutes(1))+"' WHERE assignment_id=3;");
        bool resubmitRejected=false; try { repo.submitAssignment(1,3,filePath); } catch(const std::runtime_error&) { resubmitRejected=true; }
        assert(resubmitRejected && countSubmissions(db)==3 && maxAttempt(db)==2);
      } // Close SQLite connection before removing the test database (Windows compatibility).
        std::remove(dbPath); std::remove(filePath);
        std::cout << "Submission workflow tests passed.\n"; return 0;
    } catch(const std::exception& e) { std::remove(dbPath); std::remove(filePath); std::cerr<<e.what()<<'\n'; return 1; }
}
