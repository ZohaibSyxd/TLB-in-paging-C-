#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OFFSET_MASK 0xFFF
#define PAGE_NUMBER_MASK 0x3FF

void parse_logical_address(unsigned int logical_address);
void task1(FILE *fp);

int main(int argc, char*argv[]){
    if (argc != 5) {
        return EXIT_FAILURE;
    }
    
    char *input_filename = NULL;
    int task_number = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0) {
            input_filename = argv[++i];
        } else if (strcmp(argv[i], "-t") == 0) {
            task_number = atoi(argv[++i]);
        } else {
            return 1;
        }
    }

    if (input_filename == NULL || task_number < 1 || task_number > 4) {
        return EXIT_FAILURE;
    }

    FILE *fp = fopen(input_filename, "r");
    if (fp == NULL) {
        perror("Error opening input file");
        return 1;
    }

    switch (task_number) {
        case 1:
            task1(fp);
            break;
        default:
            fprintf(stderr, "Invalid task number.\n");
            break;
    }

    fclose(fp);
    return 0;
}

// === TASK 1 ===
// Print page number and offset
void task1(FILE *fp) {
    unsigned int address;
    while (fscanf(fp, "%u", &address) == 1) {
        parse_logical_address(address);
    }
}

void parse_logical_address(unsigned int logical_address) {

    unsigned int page_number = (logical_address >> 12) & PAGE_NUMBER_MASK;
    unsigned int offset = logical_address & OFFSET_MASK;

    printf("Logical Address:%u,page-number=%u,offset=%u\n", logical_address, page_number, offset);
}