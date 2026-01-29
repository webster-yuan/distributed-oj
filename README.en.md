# distributed-oj

A load-balanced Online Judge (OJ) system developed in C++. It receives code submissions from users, performs distributed compilation and execution on the backend, and returns the results.

## Features

*   **Distributed Load Balancing**: `oj_server` acts as a task dispatcher, dynamically monitoring the load of multiple `compile_server` backends (CPU usage, memory consumption, etc.) and distributing tasks to the most suitable node using a weighted round-robin-like algorithm.
*   **Decoupled Architecture**: The judging logic is separated from the business logic. `compile_server` focuses on code compilation and execution, while `oj_server` handles user interaction, problem management, and task scheduling.
*   **Resource Limits & Security**: The backend uses system calls like `setrlimit` to restrict CPU time and memory usage when running user code, preventing malicious code from exhausting system resources.
*   **Comprehensive Business Functions**: Supports user registration, login (with salted MD5 encryption), problem CRUD (Create, Read, Update, Delete), progress tracking, and an admin dashboard.
*   **MVC Architecture**: `oj_server` implements the MVC pattern internally, using `ctemplate` for dynamic HTML rendering.

## Core Modules

### 1. `oj_server` (Business & Scheduling)
*   **MVC Implementation**:
    *   **Model**: Encapsulates MySQL database interactions for user info, problem data, and progress records.
    *   **View**: Renders HTML templates based on `ctemplate`.
    *   **Control**: The core business logic that handles HTTP requests and coordinates with the load balancer.
*   **Load Balancer (LoadBalance)**:
    *   Loads the backend server list from `service_machine.conf`.
    *   Performs real-time health checks on backend machines.
    *   Intelligently schedules tasks based on a load score (calculated from CPU, memory, active connections, etc.).

### 2. `compile_server` (Judging & Execution)
*   **Compiler Submodule**: Asynchronously compiles user code using `g++` and captures compilation errors.
*   **Runner Submodule**: Creates child processes via `fork` and uses `setrlimit` to restrict resources while running the compiled executable.
*   **Output Extraction**: Redirects stdout and stderr to temporary files, parses them, and returns the result to the scheduler.

## Tech Stack

*   **Language**: C++ 11
*   **Libraries**: `cpp-httplib` (HTTP framework), `jsoncpp` (JSON parsing), `ctemplate` (Template engine)
*   **Database**: MySQL
*   **Security**: MD5 + Salted encryption
*   **System Tools**: `fork`, `exec`, `waitpid`, `setrlimit`, `signal`

## Directory Structure

```text
.
├── comm/                   # Common utilities (logging, file ops, string manipulation, networking)
├── compile_server/         # Backend compilation and execution server
├── oj_server/              # Business and load balancing scheduler server
│   ├── conf/               # Configuration files
│   ├── questions/          # Problem data (local backup or initial data)
│   ├── template_html/      # HTML template files
│   └── wwwroot/            # Frontend static resources
├── sql/                    # SQL scripts for database setup
├── testCode/               # Test code for various components
└── makefile                # Main project build script
```

## Quick Start

### 1. Prerequisites
Ensure the following libraries are installed:
- `g++` (C++11 support)
- `mysqlclient`
- `jsoncpp`
- `ctemplate`
- `libssl` & `libcrypto` (OpenSSL)

### 2. Database Setup
Execute the SQL scripts in the `sql/` directory to create the necessary tables:
```bash
mysql -u root -p < sql/User.sql
mysql -u root -p < sql/question.sql
# ... execute other scripts in order
```

### 3. Build & Run
```bash
# Compile the entire project from the root directory
make

# Start the backend compilation service (multiple instances allowed on different ports)
cd compile_server
./compile_server 8081

# Start the business and scheduling server
cd ../oj_server
./oj_server
```

### 4. Access
Open your browser and navigate to `http://localhost:8180`.

## Future Roadmap
*   [ ] Introduce Docker containerization for better judging isolation.
*   [ ] Support more programming languages (Java, Python, Go, etc.).
*   [ ] Add Contest Mode (ACM style).
*   [ ] Optimize the frontend UI for a better user experience.
