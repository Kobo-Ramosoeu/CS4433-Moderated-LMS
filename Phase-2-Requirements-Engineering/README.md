# Phase 2: Requirements Engineering

## Overview

This phase focuses on requirements engineering for the Moderated Learning Management System (LMS) developed for the CS4433 Software Engineering assignment.

The purpose of this phase is to identify stakeholder needs, resolve conflicting requirements, define the system's functional and non-functional requirements, and model the system's interactions with its users.

## Contents

- **Stakeholder Analysis and Conflict Resolution:** Identifies the system stakeholders, their interests, conflicts between their requirements, and the proposed resolutions.
- **Software Requirements Specification (SRS):** Defines the functional and non-functional requirements, priorities, and verification methods.
- **Use Case Diagram:** Illustrates the interactions between the four system actors and the Moderated LMS.
- **Fully Dressed Use Case Specifications:** Describes the workflows, preconditions, postconditions, alternative flows, exceptions, and business rules for:
  1. Submit Assignment
  2. Mark Submission
  3. Moderate and Release Marks

## Main Stakeholders

The system considers the needs of the Principal, Head of Department (HoD), IT Officer, Student, Lecturer, and Administrator.

Key concerns include ease of use, controlled release of marks, assignment submissions and resubmissions, individual extensions, late-submission penalties, written feedback, offline operation, password protection, and performance.

## System Actors

The four primary system actors are:

- **Administrator:** Manages user accounts, departments, courses, enrolments, and lecturer assignments.
- **Lecturer:** Creates assignments, grants individual extensions, marks submissions, and provides written feedback.
- **Head of Department (HoD):** Moderates marks and releases approved results.
- **Student:** Views enrolled courses and assignments, submits or resubmits work, and views released marks and feedback.

## Key Requirements Decisions

- Students cannot view marks until the HoD has reviewed and released them.
- Late submissions are governed by the specified penalty rules.
- Individual extensions affect the applicable submission deadline.
- The system is designed to operate offline.
- Role-specific workflows and automated rules help keep the system simple for non-technical users.
- Mark changes must be recorded in an audit log.

## Deliverable

The complete Phase 2 report is available in [`Phase 2_Requirements Engineering.pdf`](./Phase%202_Requirements%20Engineering.pdf).

## Project Status

- [x] Phase 1: Project Planning and Management
- [x] Phase 2: Requirements Engineering
- [ ] Phase 3: System Design
- [ ] Subsequent implementation, testing, and documentation phases

## Technology Context

The overall assignment specifies C++17, CMake, SQLite, Git, UML modelling, a unit-testing framework, and a static-analysis tool. The implementation and design phases will use the approved requirements established in this phase as their basis.

