#ifndef LMS_MARKING_REPOSITORY_H
#define LMS_MARKING_REPOSITORY_H

#include "lms/Database.h"

#include <string>
#include <vector>

namespace lms {

struct SubmissionToMark {
    int submissionId{};
    int assignmentId{};
    int studentId{};
    int attemptNumber{};
    std::string assignmentTitle;
    std::string studentNumber;
    std::string submittedAt;
    double maxMark{};
};

struct MarkingResult {
    int daysLate{};
    double rawMark{};
    double penalty{};
    double finalMark{};
};

struct StudentReleasedMark {
    std::string assignmentTitle;
    std::string courseCode;
    double rawMark{};
    double finalMark{};
    double maxMark{};
    std::string feedback;
    std::string markedAt;
};

struct ReturnedMark {
    int markId{};
    int submissionId{};
    std::string assignmentTitle;
    std::string studentNumber;
    double rawMark{};
    double finalMark{};
    double maxMark{};
    std::string feedback;
    std::string correctionReason;
};

class MarkingRepository {
private:
    Database& database;

public:
    explicit MarkingRepository(Database& database);

    std::vector<StudentReleasedMark> getReleasedMarksForStudent(int studentId);

    std::vector<SubmissionToMark> getSubmissionsForLecturer(
        int lecturerId
    );

    std::vector<ReturnedMark> getReturnedMarksForLecturer(int lecturerId);

    MarkingResult correctReturnedMark(
        int lecturerId,
        int markId,
        double rawMark,
        const std::string& feedback
    );

    MarkingResult enterMark(
        int lecturerId,
        int submissionId,
        double rawMark,
        const std::string& feedback
    );
};

}

#endif