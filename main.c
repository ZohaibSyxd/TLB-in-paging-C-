#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OFFSET_MASK 0xFFF
#define PAGE_NUMBER_MASK 0x3FF
#define PAGE_TABLE_SIZE 1024
#define OFFSET_BITS 12

void parse_logical_address(unsigned int logical_address, unsigned int *page_number, unsigned int *offset);
void task1(FILE *fp);
void task2(FILE *fp);

typedef struct {
    int present;
    unsigned int frame_number;
} page_table_entry_t;


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
        case 2:
            task2(fp);
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
    unsigned int page_number;
    unsigned int offset;

    while (fscanf(fp, "%u", &address) == 1) {
        parse_logical_address(address, &page_number, &offset);
        // Print the parsed logical address information
        printf("Logical Address=%u,page-number=%u,offset=%u\n", address, page_number, offset);
    }
}


// === TASK 2 ===
// Page Table Implementation and Initial Frame Al-location
void task2(FILE *fp) {
    page_table_entry_t page_table[PAGE_TABLE_SIZE] = {0}; // Initialize all entries to {0, 0}
    unsigned int next_free_frame = 0; // Keep track of next free frame

    unsigned int address;
    unsigned int page_number;
    unsigned int offset;

    while (fscanf(fp, "%u", &address) == 1) {
        // Print task 1 output
        parse_logical_address(address, &page_number, &offset);
        printf("logical-address=%u,page-number=%u,offset=%u\n", address, page_number, offset);

        // Assume no page fault initialy
        int page_fault = 0;
        unsigned int frame_number;

        if (page_table[page_number].present) { // present == 1
            // Page is already loaded into memory
            frame_number = page_table[page_number].frame_number;
            page_fault = 0;
        } else { // present == 0
            // Page fault: load into memory and allocate next free frame
            page_fault = 1;
            frame_number = next_free_frame;
            page_table[page_number].present = 1;
            page_table[page_number].frame_number = next_free_frame;
            next_free_frame++;
        }

        // Extract the physical address:
        // - Shift left by 12 bits
        // - Add the offset into the lower 12 bits of the address
        unsigned int physical_address = (frame_number << OFFSET_BITS) | offset;
        
        printf("page-number=%u,page-fault=%d,frame-number=%u,physical-address=%u\n",
               page_number, page_fault, frame_number, physical_address);
    }
}



// Function to parse a logical address into page number and offset
void parse_logical_address(unsigned int logical_address, unsigned int *page_number, unsigned int *offset) {
    // Mask the logical address to keep only the lower 22 bits
    unsigned int address = logical_address & 0x003FFFFF;

    // Extract the page number:
    // - Shift right by 12 bits to remove the offset bits
    // - Apply PAGE_NUMBER_MASK to isolate only the page number bits
    *page_number = (address >> 12) & PAGE_NUMBER_MASK;

    // Extract the offset:
    // - Mask with OFFSET_MASK to keep only the lower 12 bits
    *offset = address & OFFSET_MASK;
}
