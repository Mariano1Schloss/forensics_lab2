#include <linux/unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <elf.h>

/* File types for d_type */
#define DT_UNKNOWN 0
#define DT_FIFO 1
#define DT_CHR 2
#define DT_DIR 4
#define DT_BLK 6
#define DT_REG 8
#define DT_LNK 10
#define DT_SOCK 12
#define BUF_SIZE 1024

#define handle_error(msg)           \
        do                          \
        {                           \
                perror(msg);        \
                exit(EXIT_FAILURE); \
        } while (0)

struct linux_dirent64
{
        long d_ino;              /* 64-bit inode number */
        long d_off;              /* 64-bit offset to next structure */
        unsigned short d_reclen; /* Size of this dirent */
        unsigned char d_type;    /* File type */
        char d_name[];           /* Filename (null-terminated) */
};

//System call with three arguments
static inline int syscall3(long long syscallnum, long long arg0, long long arg1, long long arg2)
{
        register long long syscallnum_ __asm__("rax");
        register long long arg0_ __asm__("rdi");
        register long long arg1_ __asm__("rsi");
        register long long arg2_ __asm__("rdx");
        syscallnum_ = syscallnum;
        arg0_ = arg0;
        arg1_ = arg1;
        arg2_ = arg2;
        asm volatile(
            "syscall"
            : "+r"(syscallnum_)
            : "r"(arg0_), "r"(arg1_), "r"(arg2_));
        return syscallnum_;
}

//wrapper for system call "getdents64"
static int getdents64(unsigned int fd, struct linux_dirent64 *dirp, unsigned int count)
{
        return syscall3(__NR_getdents64, fd, (long long)dirp, count);
}

//Check the first bytes of a file to check if it is an ELF
int is_elf_file(const char *filename)
{
        FILE *file = fopen(filename, "rb");
        if (!file)
        {
                handle_error("Failed to open file");
                return 0;
        }

        uint8_t magic[4];
        if (fread(magic, sizeof(uint8_t), 4, file) != 4)
        {
                fclose(file);
                return 0;
        }

        fclose(file);

        // Check for the ELF magic number
        if (magic[0] == 0x7F && magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F')
        {
                return 1; // It's an ELF file
        }
        else
        {
                return 0; // It's not an ELF file
        }
}

//Display ELF architecture and version
void elf_info(const char *filename)
{
        FILE *file = fopen(filename, "r");
        if (!file)
        {
                handle_error("Failed to open file");
        }

        if (fseek(file, 4, SEEK_SET) != 0)
        {
                fclose(file);
                perror("Failed to seek to the 5th byte");
        }

        // Read the 5th byte
        unsigned char byte5;
        if (fread(&byte5, sizeof(unsigned char), 1, file) != 1)
        {
                perror("Failed to read the 5th byte");
                fclose(file);
        }

        if (fseek(file, 6, SEEK_SET) != 0)
        {
                perror("Failed to seek to the 7th byte");
                fclose(file);
        }
        // Read the 7th byte
        unsigned char byte7;
        if (fread(&byte7, sizeof(unsigned char), 1, file) != 1)
        {
                fclose(file);
                perror("Failed to read the 7th byte");
        }
        fclose(file);
        printf("   %s ", (byte5 == 1) ? "32 bits" : (byte5 == 2) ? "64 bits"
                                                                 : "???");

        printf(" %s ", (byte7 == 1) ? "Current\n" : "???\n");
}

int main(int argc, char *argv[])
{
        int fd, nread;
        char buf[BUF_SIZE];
        struct linux_dirent64 *d;
        int bpos;
        char d_type;

        fd = open(argc > 1 ? argv[1] : ".", O_RDONLY | O_DIRECTORY);
        if (fd == -1)
                handle_error("open");
        for (;;)
        {
                nread = getdents64(fd, (struct linux_dirent64 *)buf, BUF_SIZE);
                if (nread == -1)
                        handle_error("getdents");

                if (nread == 0)
                        break;

                printf("--------------- nread=%d ---------------\n", nread);
                printf("inode#   file type    d_name         is ELF   format   version  \n");
                for (bpos = 0; bpos < nread;)
                {
                        d = (struct linux_dirent64 *)(buf + bpos);
                        printf("%8ld  ", d->d_ino);
                        d_type = d->d_type;
                        printf("%-10s ", (d_type == DT_REG) ? "regular" : (d_type == DT_DIR) ? "directory"
                                                                      : (d_type == DT_FIFO)  ? "FIFO"
                                                                      : (d_type == DT_SOCK)  ? "socket"
                                                                      : (d_type == DT_LNK)   ? "symlink"
                                                                      : (d_type == DT_BLK)   ? "block dev"
                                                                      : (d_type == DT_CHR)   ? "char dev"
                                                                                             : "???");
                        printf(" %-16s", d->d_name);
                        if (d->d_type == DT_REG)
                        {
                                if (is_elf_file(d->d_name))
                                {
                                        printf(" yes ");
                                        elf_info(d->d_name);
                                }
                        }
                        printf("\n");
                        bpos += d->d_reclen;
                }
        }

        exit(EXIT_SUCCESS);
}
