# ELF structure LAB

##  Myprogram Introduction

This is a simple command-line tool that provides information about an ELF file. It is designed to display  information about file header, program header tables, and sections header tables.

## Myprogram Usage

- `-h`: Display file header information.
- `-p`: Display program header table information.
- `-s`: Display sections header table information.

## repo_parcours Introduction
The purpose of this section is to traverse a directory. For each found file, the following information should be displayed:

- Inode number
- Type (Regular: DT_REG or Directory: DT_DIR)
- Whether it is an ELF file or not (in the case of a regular file)
- The name of the file

## repo_parcours algorithm

```

**repo_parcours Algorithm:**

1. If the program is called with the name of a directory, it should list the contents of that directory. Otherwise, it should list the contents of the current directory (".").
   
2. The `open` system call is be used with the `O_RDONLY | O_DIRECTORY` mode to open the directory.

3. The open modes are defined in the `fcntl.h` header file.

4. As long as `getdents64` does not return a negative value (<=0), the following is performed on the returned entries

5. For each entry, its characteristics (inode, type, and name) are  displayed.

6. If an entry corresponds to a regular file, it is opened, and its header is read to check if it's an ELF file or not.

7. If it's an ELF file, it is displayed whether it's ELF32 or ELF64. It is also indicated if the current version is EV_CURRENT or not.
