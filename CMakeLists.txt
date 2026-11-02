cmake_minimum_required(VERSION 3.20)

project(ModeratedLMS VERSION 1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SQLite3 REQUIRED)

# The console application uses Windows BCrypt and Argon2. On non-Windows hosts,
# configure the repository tests without trying to compile the Windows-only app.
if(WIN32)
    set(MSYS2_ROOT "C:/msys64/ucrt64" CACHE PATH "MSYS2 UCRT64 installation")
    find_path(ARGON2_INCLUDE_DIR argon2.h PATHS "${MSYS2_ROOT}/include" REQUIRED)
    find_library(ARGON2_LIBRARY NAMES argon2 PATHS "${MSYS2_ROOT}/lib" REQUIRED)
    add_executable(lms
        src/main.cpp src/DeadlineCalculator.cpp src/User.cpp src/PasswordHasher.cpp
        src/data/Database.cpp src/models/Student.cpp src/models/Lecturer.cpp
        src/models/HeadOfDepartment.cpp src/models/Administrator.cpp
        src/data/UserRepository.cpp src/models/Assignment.cpp src/data/AssignmentRepository.cpp
        src/data/SubmissionRepository.cpp src/data/DeadlineExtensionRepository.cpp
        src/data/MarkingRepository.cpp src/data/ModerationRepository.cpp
        src/data/AdministrationRepository.cpp)
    target_include_directories(lms PRIVATE include "${ARGON2_INCLUDE_DIR}")
    target_link_libraries(lms PRIVATE SQLite::SQLite3 "${ARGON2_LIBRARY}" bcrypt)
else()
    message(STATUS "Non-Windows host: full app skipped (Windows BCrypt); repository tests remain available.")
endif()
include(CTest)
if(BUILD_TESTING)
    add_executable(deadline_tests
        test_deadline.cpp
        src/DeadlineCalculator.cpp
        src/data/DeadlineExtensionRepository.cpp
        src/data/Database.cpp
    )
    target_include_directories(deadline_tests PRIVATE include)
    target_link_libraries(deadline_tests PRIVATE SQLite::SQLite3)
    add_test(NAME late_penalty_boundaries COMMAND deadline_tests)

    add_executable(repository_integration_tests
        Phase-4-Implementation/tests/repository_integration_tests.cpp
        src/data/Database.cpp
        src/data/DeadlineExtensionRepository.cpp
        src/data/AdministrationRepository.cpp
    )
    target_include_directories(repository_integration_tests PRIVATE include)
    target_link_libraries(repository_integration_tests PRIVATE SQLite::SQLite3)
    add_test(NAME repository_integration COMMAND repository_integration_tests)

    add_executable(submission_workflow_tests
        Phase-4-Implementation/tests/submission_workflow_tests.cpp
        src/data/Database.cpp
        src/data/SubmissionRepository.cpp
        src/data/DeadlineExtensionRepository.cpp
    )
    target_include_directories(submission_workflow_tests PRIVATE include)
    target_link_libraries(submission_workflow_tests PRIVATE SQLite::SQLite3)
    add_test(NAME submission_workflow COMMAND submission_workflow_tests)

    add_executable(mark_moderation_release_tests
        Phase-4-Implementation/tests/mark_moderation_release_tests.cpp
        src/data/Database.cpp
        src/data/MarkingRepository.cpp
        src/data/ModerationRepository.cpp
        src/DeadlineCalculator.cpp
        src/data/DeadlineExtensionRepository.cpp
    )
    target_include_directories(mark_moderation_release_tests PRIVATE include)
    target_link_libraries(mark_moderation_release_tests PRIVATE SQLite::SQLite3)
    add_test(NAME mark_moderation_release COMMAND mark_moderation_release_tests)

    # Tests load the schema from the source tree and create disposable DB files.
    set_tests_properties(late_penalty_boundaries repository_integration submission_workflow mark_moderation_release
        PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endif()
