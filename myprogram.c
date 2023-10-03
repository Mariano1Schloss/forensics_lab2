#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// display file header information whith option -h
void *fileHeader(FILE *filePointer)
{
	Elf64_Ehdr elfHeader;
	if (fread(&elfHeader, 1, sizeof(Elf64_Ehdr), filePointer) != sizeof(Elf64_Ehdr))
	{
		perror("Error reading ELF header");
	}

	unsigned char *magicNumber = elfHeader.e_ident;
	Elf64_Half file_type = elfHeader.e_type;
	Elf64_Word file_version = elfHeader.e_version;
	printf("file magic number : %s\n", magicNumber);
	if (file_type != ET_DYN && file_type != ET_DYN)
	{
		perror("Wrond file type \n");
	}
	if (file_version != EV_CURRENT)
	{
		perror("Wrong version\n");
	}

	printf("entry point : 0x%lx \n", elfHeader.e_entry);
	printf("Program header table : Size: %u (bytes)  Entry count: %u Offset: %lu (bytes into file) \n", elfHeader.e_phentsize, elfHeader.e_phnum, elfHeader.e_phoff);
	printf("\nSection header table : Size: %u (bytes)  Entry count: %u Offset: %lu (bytes into file) \n", elfHeader.e_shentsize, elfHeader.e_shnum, elfHeader.e_shoff);
}

/*display program header information whith option -p

**Get the program header table offset and number of entries
**Allocate the right amount of memory to *phHeaders
**Set the file cursor to the pht offset
**Read the pht into the *phHeaders variable
**Display the required information

*/
void *programHeader(FILE *filePointer)
{
	Elf64_Ehdr elfHeader;
	if (fread(&elfHeader, 1, sizeof(Elf64_Ehdr), filePointer) != sizeof(Elf64_Ehdr))
	{
		perror("Error reading ELF header");
	}
	Elf64_Off phoff = elfHeader.e_phoff;
	Elf64_Half phnum = elfHeader.e_phnum;
	Elf64_Phdr *phHeaders = (Elf64_Phdr *)malloc(sizeof(Elf64_Phdr) * phnum);
	if (!phHeaders)
	{
		perror("malloc");
		exit(1);
	}
	if (fseek(filePointer, phoff, SEEK_SET) != 0)
	{
		perror("fseek");
		exit(1);
	}
	if (fread(phHeaders, 1, sizeof(Elf64_Phdr) * phnum, filePointer) != sizeof(Elf64_Phdr) * phnum)
	{
		perror("Error reading Program header table");
	}
	for (int i = 0; i < phnum; i++)
	{
		Elf64_Phdr *phdr = &phHeaders[i];
		// Check if this segment should be loaded
		if (phdr->p_type == PT_LOAD)
		{
			printf("\nSegment %d:\n", i);
			printf("  Type: %u\n", phdr->p_type);
			printf("  Flags: %u\n", phdr->p_flags);
			printf("  File Offset: 0x%lx\n", phdr->p_offset);
			printf("  Virtual Address: 0x%lx\n", phdr->p_vaddr);
			printf("  Physical Address: 0x%lx\n", phdr->p_paddr);
			printf("  File Size: 0x%lx bytes\n", phdr->p_filesz);
			printf("  Memory Size: 0x%lx bytes\n", phdr->p_memsz);
			printf("  Alignment: 0x%lx\n", phdr->p_align);
		}
	}
	free(phHeaders);
}

/*Display section header table as well as string table content with option -s

**Get the section header table offset and number of entries
**Allocate the right amount of memory to *shHeaders
**Set the file cursor to the sht offset
**Read the sht into the *shHeaders variable
** Locate .strtable section with the e_shstrndx file header paramater
** Read .strtable section into strtable variable
** Split the string table into individual strings
**Display the required information about the sections, with their name obtained with the strtable and the section index

*/
void *sectionHeader(FILE *filePointer)
{
	Elf64_Ehdr elfHeader;

	if (fread(&elfHeader, 1, sizeof(Elf64_Ehdr), filePointer) != sizeof(Elf64_Ehdr))
	{
		perror("Error reading ELF header");
	}

	Elf64_Off shoff = elfHeader.e_shoff;
	Elf64_Half shnum = elfHeader.e_shnum;
	Elf64_Shdr *shHeaders = (Elf64_Shdr *)malloc(sizeof(Elf64_Shdr) * shnum);

	if (!shHeaders)
	{
		perror("malloc");
		exit(1);
	}
	if (fseek(filePointer, shoff, SEEK_SET) != 0)
	{
		perror("fseek");
		exit(1);
	}
	if (fread(shHeaders, 1, sizeof(Elf64_Shdr) * shnum, filePointer) != sizeof(Elf64_Shdr) * shnum)
	{
		perror("Error reading Section header table");
	}

	Elf64_Shdr *sh_strtable = &shHeaders[elfHeader.e_shstrndx];
	Elf64_Off strtable_offset = sh_strtable->sh_offset;
	Elf64_Half strtable_size = sh_strtable->sh_size;

	if (fseek(filePointer, strtable_offset, SEEK_SET) != 0)
	{
		perror("fseek");
		exit(1);
	}
	char strtable[strtable_size];
	if (fread(strtable, 1, strtable_size, filePointer) != strtable_size)
	{
		perror("Error reading string table");
	}
	printf("Section .strtable Address: 0x%lx\n", (unsigned long)sh_strtable->sh_addr);
	printf("Section .strtable Offset: 0x%lx\n", (unsigned long)strtable_offset);
	printf("Section .strtable Size: %lu bytes\n", (unsigned long)strtable_size);
	printf("String .strtable Contents:\n");

	// Split the string table into individual strings
	char *string = strtable;
	for (size_t i = 0; i < strtable_size; i++)
	{
		if (strtable[i] == '\0')
		{
			printf("String %zu: %s\n", i, string);
			string = &strtable[i + 1];
		}
	}
	for (int i = 0; i < shnum; i++)
	{
		Elf64_Shdr *shdr = &shHeaders[i];
		printf("Section Position : %u    %s\n", shdr->sh_name, &strtable[shdr->sh_name]);
		printf("Virtual Address of the Section during Execution: 0x%lx\n", shdr->sh_addr);
		printf("Position of the Section in the File: 0x%lx\n\n", shdr->sh_offset);
	}
	free(shHeaders);
}

void displayHelp()
{
	printf("Options:\n");
	printf("  -h    Display file header information\n");
	printf("  -p    Display program header table information\n");
	printf("  -s    Display sections header table information\n");
}

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		printf("Insufficient arguments.\n");
		displayHelp();
		return 1;
	}

	const char *filename = argv[argc - 1];
	FILE *filePointer = fopen(filename, "r");
	if (filePointer == NULL)
	{
		perror("Error opening file");
	}

	if (strcmp(argv[1], "-h") == 0)
	{
		fileHeader(filePointer);
	}
	else if (strcmp(argv[1], "-p") == 0)
	{
		programHeader(filePointer);
	}
	else if (strcmp(argv[1], "-s") == 0)
	{
		sectionHeader(filePointer);
	}
	else
	{
		printf("Invalid option: %s\n", argv[1]);
		displayHelp();
		fclose(filePointer);
		return 1;
	}

	fclose(filePointer);
	return 0;
}
