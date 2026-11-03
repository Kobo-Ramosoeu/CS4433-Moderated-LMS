#include "lms/models/Assignment.h"

namespace lms {

Assignment::Assignment(
    int assignmentId,
    int courseId,
    const std::string& title,
    const std::string& description,
    const std::string& deadline,
    double maxMark
)
    : assignmentId(assignmentId),
      courseId(courseId),
      title(title),
      description(description),
      deadline(deadline),
      maxMark(maxMark)
{
}

int Assignment::getAssignmentId() const
{
    return assignmentId;
}

int Assignment::getCourseId() const
{
    return courseId;
}

const std::string& Assignment::getTitle() const
{
    return title;
}

const std::string& Assignment::getDescription() const
{
    return description;
}

const std::string& Assignment::getDeadline() const
{
    return deadline;
}

double Assignment::getMaxMark() const
{
    return maxMark;
}

}