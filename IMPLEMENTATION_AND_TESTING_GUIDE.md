# Installation and final integration guide

1. Make a backup of the current `lms.db` in the project root.
2. Extract this source package into your existing `CS4433-Moderated-LMS` folder, allowing source files to be replaced. The package deliberately excludes `lms.db`, `database/lms.db`, `.git`, and build output.
3. Open PowerShell in the project root.
4. Build:

   ```powershell
   & "C:\Program Files\CMake\bin\cmake.exe" -S . -B build-phase4 -G "MinGW Makefiles"
   & "C:\Program Files\CMake\bin\cmake.exe" --build build-phase4
   ```

5. Run:

   ```powershell
   .\build-phase4\lms.exe
   ```

6. If the database has no Administrator account, create the first Administrator when prompted. Use that account to create Student, Lecturer, HoD and other Administrator accounts.
7. Create departments and courses; assign a HoD to each department; assign Lecturers to courses; enrol Students in courses.
8. Keep the backup until the application has been fully tested.

## Existing database migration

On startup, `database/schema.sql` creates missing schema objects. The application then checks whether the `User.is_active` column exists and adds it when upgrading an older database. Audit immutability triggers are also installed. Existing records are not intentionally deleted or recreated.

## Suggested complete workflow test after installation

1. Administrator: create a department, course, HoD, Lecturer and Student; assign the HoD and Lecturer; enrol the Student.
2. Lecturer: create an assignment; grant one student an extension; mark a submission with feedback.
3. HoD: review the mark; adjust it with a reason or approve it; release approved marks.
4. Student: verify enrolled courses and assignment visibility; submit/resubmit according to deadline rules; confirm marks are visible only after release.
5. Access control: create a second student and verify that each student sees only their own released marks.
6. Verify audit records and confirm UPDATE/DELETE against `AuditRecord` are rejected by the database triggers.
