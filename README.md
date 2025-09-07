# Git Command Line Interface

[![Progress Banner](https://backend.codecrafters.io/progress/git/f5a75a5c-fb7c-4664-b9c5-68ae072639e1)](https://app.codecrafters.io/users/codecrafters-bot?r=2qF)

## Table of Contents
- [Introduction](#introduction)
- [Prerequisites](#prerequisites)
- [Quickstart](#quickstart)
- [Commands](#commands)

## Introduction
Git commands are broadly divided into two types:
1. **Porcelain Commands**: High-level commands that users interact with directly (e.g., `git add`, `git commit`, `git push`)  
2. **Plumbing Commands**: Low-level commands that interact with the Git database (e.g., `git hash-object`, `git cat-file`, `git show-ref`)

`git_cli` is a lightweight Git implementation in C++, focusing on core plumbing commands. Explore how Git manages objects (blobs, trees, commits) and repository snapshots under the hood.

## Prerequisites
`git_cli` is designed for Unix-like systems (Linux, macOS, or WSL). Before building and using the project, the following tools are required:
- CMake (version ≥ 3.28)
- C++ compiler with C++17/23 support (e.g., g++ or clang++)
- Make (build-essential on Ubuntu/Debian)
- Git
- Bash
- Unzip – needed to extract the SHA1 library
  ```
  sudo apt update
  sudo apt install unzip
  ```
### Installing Dependencies
The repository includes a helper script to set up the required SHA1/Zlib library:
```
bash install-sha1.sh
```
This downloads and extracts the library locally into the project, so no system-wide installation of Zlib is needed.
> **Note:** Windows without WSL is not currently supported.

## Quickstart

1. **Clone the repository:**
    ```
    https://github.com/jenneva-li/git_cli.git
    cd git_cli
    ```
2. **Create and configure the build directory:**
    ```
    mkdir build
    cmake -B build
    ```
3. **Build the project:**
   ```
   cmake --build build
   ```
4. **Install `git_cli` to `~/.local/bin` for global access:**
   ```
   cmake --install build --prefix ~/.local
   echo 'export PATH=$HOME/.local/bin:$PATH' >> ~/.bashrc
   source ~/.bashrc
   ```
## Commands:
### `init`
Initialize a new Git repository in the current directory (or a specified path).
```
git_cli init
# or
git_cli init /path/to/repo
```
### `cat-file`
Show information about a Git object.
```
git_cli cat-file -p <object>    # Print object content
git_cli cat-file -t <object>    # Print object type
git_cli cat-file -s <object>    # Print object size
```
**Example:**
```
git_cli cat-file -p 0fc555c
```
### `hash-object`
Compute the SHA1 hash of a file and optionally store it in the object database.
```
git_cli hash-object [-t <type>] [-w] [--path=<file>]
```
- `-t <type>` — specify object type (`blob`, `tree`, etc.)
- `-w` — write the object into the database
  
**Example:**
```
git_cli hash-object -t blob -w file.txt
```
### `log`
Display the commit history.
```
git_cli log <commit-sha>
```
**Example:**
```
git_cli log 0fc555ccba3fa699e194be79259f7161
```
### `ls-tree`
List the contents of a tree object.
```
git_cli ls-tree [options] <tree-ish> [path]
```
**Options:**
- `-r` — recursive
- `--name only` — only show file names
- `--long` — show detailed info (mode, type, SHA, size, path)
  
**Example:**
```
git_cli ls-tree -r 4a7d1f
git_cli ls-tree --name-only 4a7d1f
```
### `checkout`
Check out a branch or commit into a target directory.
```
git_cli checkout <branch> [target-path]
```
**Example:**
```
git_cli checkout main
git_cli checkout main /tmp/myrepo
```
