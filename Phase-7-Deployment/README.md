# Phase 7 — Deployment

This phase documents how to prepare, build, configure, launch and hand over the Moderated LMS application.

## Deployment scope
The current deliverable is a local/offline Windows console application written in C++17, built with CMake and using SQLite. It is not a hosted web service.

## Contents
- [Deployment and Installation Guide](docs/Deployment-and-Installation-Guide.md)
- [User Guide](docs/User-Guide.md)
- [Deployment Verification Checklist](docs/Deployment-Verification-Checklist.md)

## Deployment boundary
The source repository is the deployment package. A working deployment also requires the supported Windows/MSYS2 UCRT64 toolchain and dependencies. The database is created/used locally and is not distributed as a populated production database. A backup of an existing `lms.db` must be made before upgrading.
