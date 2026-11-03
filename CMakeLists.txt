
cmake_minimum_required(VERSION 3.20)

project(ModeratedLMS VERSION 1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SQLite3 REQUIRED)

# Enable GCC/Clang coverage instrumentation when requested.
option(LMS_ENABLE_COVERAGE "Enable code coverage for tests" OFF)

function(enable_target_coverage target_name)
    if(LMS_ENABLE_COVERAGE)
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(${target_name} PRIVATE
                --coverage
                -O0
                -g
            )

            target_link_options(${target_name} PRIVATE
                --coverage
            )
        else()
            message(FATAL_ERROR
                "Coverage is supported here only with GCC or Clang."
            )
        endif()
    endif()
endfunction()

# The console application uses Windows BCrypt and Argon2.
# On non-Windows hosts, build the repository tests only.
if(WIN32)
    set(MSYS2_ROOT "C:/msys64/ucrt64" CACHE PATH "MSYS2 UCRT64 installation")

    find_path(ARGON2_INCLUDE_DIR argon2.h
        PATHS "${MSYS2_ROOT}/include"
        REQUIRED
    )

    find_library(ARGON2_LIBRARY argon2
        PATHS "${MSYS2_ROOT}/lib"
        REQUIRED
    )

    add_executable(lms
        src/main.cpp
        src/DeadlineCalculator.cpp
        src/User.cpp
        src/PasswordHasher.cpp
        src/data/Database.cpp
        src/models/Student.cpp
        src/models/Lecturer.cpp
        src/models/HeadOfDepartment.cpp
        src/models/Administrator.cpp
        src/data/UserRepository.cpp
        src/models/Assignment.cpp
        src/data/AssignmentRepository.cpp
        src/data/SubmissionRepository.cpp
        src/data/DeadlineExtensionRepository.cpp
        src/data/MarkingRepository.cpp
        src/data/ModerationRepository.cpp
        src/data/AdministrationRepository.cpp
    )

    target_include_directories(lms PRIVATE include "${ARGON2_INCLUDE_DIR}")

    target_link_libraries(lms PRIVATE
        SQLite::SQLite3
        "${ARGON2_LIBRARY}"
        bcrypt
    )
else()
    message(STATUS
        "Non-Windows host: full app skipped (Windows BCrypt); repository tests remain available."
    )
endif()

# --------------------------------------------------
# Testing configuration
# --------------------------------------------------

include(CTest)

if(BUILD_TESTING)

    # Catch2 testing framework
    include(FetchContent)

    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.8.1
        GIT_SHALLOW TRUE
    )

    FetchContent_MakeAvailable(Catch2)

    include(Catch)

    # 1. Deadline calculator unit tests
    add_executable(deadline_calculator_unit_tests
        Phase-4-Implementation/tests/deadline_calculator_unit_tests.cpp
        src/DeadlineCalculator.cpp
        src/data/DeadlineExtensionRepository.cpp
        src/data/Database.cpp
    )

    target_include_directories(deadline_calculator_unit_tests PRIVATE include)

    target_link_libraries(deadline_calculator_unit_tests PRIVATE
        Catch2::Catch2WithMain
        SQLite::SQLite3
    )

    catch_discover_tests(deadline_calculator_unit_tests)
    enable_target_coverage(deadline_calculator_unit_tests)

    # 2. Existing deadline boundary tests
    add_executable(deadline_tests
        test_deadline.cpp
        src/DeadlineCalculator.cpp
        src/data/DeadlineExtensionRepository.cpp
        src/data/Database.cpp
    )

    target_include_directories(deadline_tests PRIVATE include)
    target_link_libraries(deadline_tests PRIVATE SQLite::SQLite3)

    add_test(NAME late_penalty_boundaries COMMAND deadline_tests)
    enable_target_coverage(deadline_tests)

    # 3. Repository integration tests
    add_executable(repository_integration_tests
        Phase-4-Implementation/tests/repository_integration_tests.cpp
        src/data/Database.cpp
        src/data/DeadlineExtensionRepository.cpp
        src/data/AdministrationRepository.cpp
    )

    target_include_directories(repository_integration_tests PRIVATE include)
    target_link_libraries(repository_integration_tests PRIVATE SQLite::SQLite3)

    add_test(NAME repository_integration COMMAND repository_integration_tests)
    enable_target_coverage(repository_integration_tests)

    # 4. Submission workflow tests
    add_executable(submission_workflow_tests
        Phase-4-Implementation/tests/submission_workflow_tests.cpp
        src/data/Database.cpp
        src/data/SubmissionRepository.cpp
        src/data/DeadlineExtensionRepository.cpp
    )

    target_include_directories(submission_workflow_tests PRIVATE include)
    target_link_libraries(submission_workflow_tests PRIVATE SQLite::SQLite3)

    add_test(NAME submission_workflow COMMAND submission_workflow_tests)
    enable_target_coverage(submission_workflow_tests)

    # 5. Marking, moderation, and release tests
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
    enable_target_coverage(mark_moderation_release_tests)

    # 6. Database integration tests
    add_executable(database_tests
        Phase-4-Implementation/tests/database_tests.cpp
        src/data/Database.cpp
    )

    target_include_directories(database_tests PRIVATE include)

    target_link_libraries(database_tests PRIVATE
        SQLite::SQLite3
    )

    add_test(NAME database_tests COMMAND database_tests)
    enable_target_coverage(database_tests)

    # Tests load the schema from the source tree and create disposable databases.
    set_tests_properties(
        late_penalty_boundaries
        repository_integration
        submission_workflow
        mark_moderation_release
        database_tests
        PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )

endif()
