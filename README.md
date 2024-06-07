
# Improved Bash Shell

## Overview

Improved Bash Shell is a custom shell program developed in C. The shell provides an interactive command-line interface for users, allowing them to execute various commands, manage processes, and utilize a range of built-in functionalities. This project showcases my skills in C programming, system calls, and process management.

## Project Status

Note: The application is fully functional. The repository serves as a comprehensive showcase of the skills and technologies utilized during its development.

## Features

- **Interactive Command Line Interface:** Users can execute standard Linux commands within the shell environment.
- **Built-in Commands:** Includes commands like `cd`, `pwd`, `exit`, `history`, and more.
- **Process Management:** Supports running commands in the background and handles process termination gracefully.
- **Command History:** Keeps a history of executed commands, allowing users to repeat previous commands easily.
- **Resource Monitoring:** Displays CPU and memory usage in the command prompt.

## Technologies Used

- **Language:** C
- **Libraries:** Standard C Libraries, POSIX Libraries

## Repository Structure

The project is organized into the following main files:

### Files

- **shell.c:** Main source file containing the implementation of the shell program.
- **Makefile:** Defines the build process for the shell program.
- **Dockerfile:** Configuration file for building a Docker image of the shell.

## Installation

### Clone the repository:

\`\`\`sh
git clone https://github.com/Sallin142/Improved-Bash-Shell.git
cd Improved-Bash-Shell
\`\`\`

### Build the Docker image:

\`\`\`sh
docker build -t a1-shell .
\`\`\`

### Run the Docker container:

\`\`\`sh
docker run -it a1-shell
\`\`\`

## Setup

To set up the application locally without Docker, follow these steps:

1. **Build the Shell:**

    \`\`\`sh
    make clean && make
    \`\`\`

2. **Run the Shell:**

    \`\`\`sh
    ./shell
    \`\`\`

## Usage

Once the shell is running, you can use it just like a regular terminal. The prompt will display the current directory along with CPU and memory usage. You can execute standard Linux commands, use built-in commands, and manage processes.

## Known Issues

There are no known issues at this time.

## Future Improvements

- **Refactoring:** Continuously improve the codebase and update libraries as needed.
- **New Features:** Potential to add more built-in commands and enhance existing functionalities.
- **Testing:** Implement comprehensive unit and integration tests to ensure robustness.

## Screenshots

![Screenshot 1](path/to/screenshot1.png)

## Contributors

- **Sallin Koutev** - [GitHub](https://github.com/Sallin142)

## Contact

For any questions or feedback, please contact me at [ska287@sfu.ca](mailto:ska287@sfu.ca).

---

© 2024 Improved Bash Shell. All rights reserved.

---

**Languages**

- C: 100%




---


