PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS "User" (
    user_id INTEGER PRIMARY KEY,
    username TEXT NOT NULL UNIQUE,
    password_hash TEXT NOT NULL,
    full_name TEXT NOT NULL,
    role TEXT NOT NULL CHECK (
        role IN ('Student', 'Lecturer', 'HoD', 'Administrator')
    ),
    is_active INTEGER NOT NULL DEFAULT 1 CHECK (is_active IN (0,1))
);

CREATE TABLE IF NOT EXISTS Department (
    department_id INTEGER PRIMARY KEY,
    name TEXT NOT NULL UNIQUE,
    hod_user_id INTEGER,
    FOREIGN KEY (hod_user_id) REFERENCES "User"(user_id)
);

CREATE TABLE IF NOT EXISTS Student (
    user_id INTEGER PRIMARY KEY,
    student_number TEXT NOT NULL UNIQUE,
    FOREIGN KEY (user_id) REFERENCES "User"(user_id)
);

CREATE TABLE IF NOT EXISTS Lecturer (
    user_id INTEGER PRIMARY KEY,
    staff_number TEXT NOT NULL UNIQUE,
    FOREIGN KEY (user_id) REFERENCES "User"(user_id)
);

CREATE TABLE IF NOT EXISTS HeadOfDepartment (
    user_id INTEGER PRIMARY KEY,
    FOREIGN KEY (user_id) REFERENCES "User"(user_id)
);

CREATE TABLE IF NOT EXISTS Course (
    course_id INTEGER PRIMARY KEY,
    department_id INTEGER NOT NULL,
    course_code TEXT NOT NULL UNIQUE,
    course_name TEXT NOT NULL,
    FOREIGN KEY (department_id) REFERENCES Department(department_id)
);

CREATE TABLE IF NOT EXISTS CourseLecturer (
    course_id INTEGER NOT NULL,
    lecturer_id INTEGER NOT NULL,
    PRIMARY KEY (course_id, lecturer_id),
    FOREIGN KEY (course_id) REFERENCES Course(course_id),
    FOREIGN KEY (lecturer_id) REFERENCES Lecturer(user_id)
);

CREATE TABLE IF NOT EXISTS Enrolment (
    student_id INTEGER NOT NULL,
    course_id INTEGER NOT NULL,
    PRIMARY KEY (student_id, course_id),
    FOREIGN KEY (student_id) REFERENCES Student(user_id),
    FOREIGN KEY (course_id) REFERENCES Course(course_id)
);

CREATE TABLE IF NOT EXISTS Assignment (
    assignment_id INTEGER PRIMARY KEY,
    course_id INTEGER NOT NULL,
    title TEXT NOT NULL,
    description TEXT,
    deadline TEXT NOT NULL,
    max_mark REAL NOT NULL CHECK (max_mark > 0),
    FOREIGN KEY (course_id) REFERENCES Course(course_id)
);

CREATE TABLE IF NOT EXISTS DeadlineExtension (
    extension_id INTEGER PRIMARY KEY,
    assignment_id INTEGER NOT NULL,
    student_id INTEGER,
    new_deadline TEXT NOT NULL,
    reason TEXT NOT NULL,
    granted_by INTEGER NOT NULL,
    FOREIGN KEY (assignment_id) REFERENCES Assignment(assignment_id),
    FOREIGN KEY (student_id) REFERENCES Student(user_id),
    FOREIGN KEY (granted_by) REFERENCES Lecturer(user_id)
);

CREATE TABLE IF NOT EXISTS Submission (
    submission_id INTEGER PRIMARY KEY,
    assignment_id INTEGER NOT NULL,
    student_id INTEGER NOT NULL,
    attempt_number INTEGER NOT NULL CHECK (attempt_number > 0),
    file_path TEXT NOT NULL,
    submitted_at TEXT NOT NULL,
    UNIQUE (assignment_id, student_id, attempt_number),
    FOREIGN KEY (assignment_id) REFERENCES Assignment(assignment_id),
    FOREIGN KEY (student_id) REFERENCES Student(user_id)
);

CREATE TABLE IF NOT EXISTS Mark (
    mark_id INTEGER PRIMARY KEY,
    submission_id INTEGER NOT NULL UNIQUE,
    raw_mark REAL NOT NULL CHECK (raw_mark >= 0),
    final_mark REAL NOT NULL CHECK (final_mark >= 0),
    feedback TEXT NOT NULL,
    status TEXT NOT NULL CHECK (
        status IN (
            'AwaitingModeration',
            'CorrectionRequired',
            'Approved',
            'Released'
        )
    ),
    marked_by INTEGER NOT NULL,
    marked_at TEXT NOT NULL,
    FOREIGN KEY (submission_id) REFERENCES Submission(submission_id),
    FOREIGN KEY (marked_by) REFERENCES Lecturer(user_id)
);

CREATE TABLE IF NOT EXISTS ModerationRecord (
    moderation_id INTEGER PRIMARY KEY,
    mark_id INTEGER NOT NULL,
    hod_id INTEGER NOT NULL,
    decision TEXT NOT NULL CHECK (
        decision IN ('Approved', 'CorrectionRequired')
    ),
    reason TEXT,
    action_at TEXT NOT NULL,
    FOREIGN KEY (mark_id) REFERENCES Mark(mark_id),
    FOREIGN KEY (hod_id) REFERENCES HeadOfDepartment(user_id)
);

CREATE TABLE IF NOT EXISTS AuditRecord (
    audit_id INTEGER PRIMARY KEY,
    user_id INTEGER NOT NULL,
    mark_id INTEGER,
    action TEXT NOT NULL,
    old_value TEXT,
    new_value TEXT,
    reason TEXT,
    action_at TEXT NOT NULL,
    FOREIGN KEY (user_id) REFERENCES "User"(user_id),
    FOREIGN KEY (mark_id) REFERENCES Mark(mark_id)
);

CREATE INDEX IF NOT EXISTS idx_assignment_course
    ON Assignment(course_id);

CREATE INDEX IF NOT EXISTS idx_submission_student_assignment
    ON Submission(student_id, assignment_id);

CREATE INDEX IF NOT EXISTS idx_mark_status
    ON Mark(status);

CREATE INDEX IF NOT EXISTS idx_audit_mark
    ON AuditRecord(mark_id);

-- Audit records are append-only: normal application SQL cannot alter or delete them.
CREATE TRIGGER IF NOT EXISTS audit_record_no_update
BEFORE UPDATE ON AuditRecord
BEGIN
    SELECT RAISE(ABORT, 'Audit records are immutable.');
END;

CREATE TRIGGER IF NOT EXISTS audit_record_no_delete
BEFORE DELETE ON AuditRecord
BEGIN
    SELECT RAISE(ABORT, 'Audit records are immutable.');
END;
