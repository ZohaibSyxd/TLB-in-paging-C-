#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OFFSET_MASK 0xFFF
#define PAGE_NUMBER_MASK 0x3FF
#define PAGE_TABLE_SIZE 1024
#define OFFSET_BITS 12
#define MAX_FRAMES 256
#define TLB_SIZE 32

void parse_logical_address(unsigned int logical_address, unsigned int *page_number, unsigned int *offset);
void task1(FILE *fp);
void task2(FILE *fp);
void task3(FILE *fp);
void task4(FILE *fp);

typedef struct {
    int present;
    unsigned int frame_number;
} page_table_entry_t;

typedef struct {
    int valid;
    unsigned int page_number;
    unsigned int frame_number;
    unsigned long last_used_time; // to implement LRU
} tlb_entry_t;

int main(int argc, char*argv[]){
    if (argc != 5) {
        return EXIT_FAILURE;
    }
    
    char *input_filename = NULL;
    char *task_name = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0) {
            input_filename = argv[++i];
        } else if (strcmp(argv[i], "-t") == 0) {
            task_name = argv[++i];
        } else {
            return 1;
        }
    }

    if (input_filename == NULL || task_name == NULL) {
        return EXIT_FAILURE;
    }

    FILE *fp = fopen(input_filename, "r");
    if (fp == NULL) {
        perror("Error opening input file");
        return 1;
    }

    if (strcmp(task_name, "task1") == 0) {
        task1(fp);
    } else if (strcmp(task_name, "task2") == 0) {
        task2(fp);
    } else if (strcmp(task_name, "task3") == 0) {
        task3(fp);
    } else if (strcmp(task_name, "task4") == 0) {
        task4(fp);
    } else {
        fprintf(stderr, "Invalid task name.\n");
        fclose(fp);
        return EXIT_FAILURE; // Exit if invalid task
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
        printf("logical-address=%u,page-number=%u,offset=%u\n", address, page_number, offset);
    }
}


// === TASK 2 ===
// Page Table Implementation and Initial Frame Al-location
void task2(FILE *fp) {
    page_table_entry_t page_table[PAGE_TABLE_SIZE] = {0};   // Initialize all entries to {0, 0}
    unsigned int next_free_frame = 0;   // Keep track of next free frame

    unsigned int address;
    unsigned int page_number;
    unsigned int offset;

    while (fscanf(fp, "%u", &address) == 1) {
        // Print task 1 output
        parse_logical_address(address, &page_number, &offset);
        printf("logical-address=%u,page-number=%u,offset=%u\n", address, page_number, offset);

        // Assume no page fault (for now)
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

// === TASK 3 ===
// FIFO Page Replacement Algorithm
void task3(FILE *fp) {
    page_table_entry_t page_table[PAGE_TABLE_SIZE] = {0};   // Initialize all entries to {0, 0}
    unsigned int page_order[MAX_FRAMES];    // Circular FIFO queue to track the order pages are loaded
    
    // FIFO pointers
    unsigned int head = 0;
    unsigned int tail = 0;

    unsigned int next_free_frame = 0;   // Keep track of next free frame
    unsigned int address;
    unsigned int page_number;
    unsigned int offset;

    // Process each logical address
    while (fscanf(fp, "%u", &address) == 1) {
        parse_logical_address(address, &page_number, &offset); // split address into page number and offset
        printf("logical-address=%u,page-number=%u,offset=%u\n", address, page_number, offset);
        
        // Assume no page fault initialy
        int page_fault = 0;                   // flag for whether a page fault occurred
        unsigned int frame_number;            // frame number where the page is loaded

        if (page_table[page_number].present) { // present == 1
            // Page is already loaded into memory
            frame_number = page_table[page_number].frame_number;
            page_fault = 0;
        } else { // present == 0
            // Page fault: load into memory and allocate next free frame
            page_fault = 1;

            if (next_free_frame < MAX_FRAMES) {
                // There is an available free frame, repeat same as task 2
                frame_number = next_free_frame;
                page_table[page_number].present = 1;
                page_table[page_number].frame_number = frame_number;
                page_order[tail] = page_number; // add page to tail of queue
                tail = (tail + 1) % MAX_FRAMES; // move tail one down for next frame and if it is at the end wrap around back to the beginning(circular)
                next_free_frame++;             // move to next free frame
            } else {
                // No free frames available: evict a page using FIFO
                unsigned int page_to_evict = page_order[head]; // page at the front of FIFO queue
                head = (head + 1) % MAX_FRAMES; // move head one down for next frame and if it is at the end wrap around back to the beginning(circular)
                unsigned int evicted_frame = page_table[page_to_evict].frame_number;

                // Print eviction information
                printf("evicted-page=%u,freed-frame=%u\n", page_to_evict, evicted_frame);

                page_table[page_to_evict].present = 0; // mark evicted page as not present

                // Load new page into the freed frame
                frame_number = evicted_frame;
                page_table[page_number].present = 1;
                page_table[page_number].frame_number = frame_number;
                page_order[tail] = page_number; // add new page to FIFO queue
                tail = (tail + 1) % MAX_FRAMES; // move tail one down for next frame and if it is at the end wrap around back to the beginning(circular)
            }
        }

        // Extract the physical address:
        // - Shift left by 12 bits
        // - Add the offset into the lower 12 bits of the address
        unsigned int physical_address = (frame_number << OFFSET_BITS) | offset;

        // Output result
        printf("page-number=%u,page-fault=%d,frame-number=%u,physical-address=%u\n",
               page_number, page_fault, frame_number, physical_address);
    }
}

void task4(FILE *fp) {
    page_table_entry_t page_table[PAGE_TABLE_SIZE] = {0}; // page table
    unsigned int page_order[MAX_FRAMES]; // FIFO queue for page replacement
    unsigned int head = 0, tail = 0;     // FIFO pointers
    unsigned int next_free_frame = 0;

    tlb_entry_t tlb[TLB_SIZE] = {0}; // Initialize TLB
    unsigned long time_counter = 0;  // Global time counter for LRU

    unsigned int address, page_number, offset;

    while (fscanf(fp, "%u", &address) == 1) {
        time_counter++; // Increase time on each memory access
        parse_logical_address(address, &page_number, &offset);
        printf("logical-address=%u,page-number=%u,offset=%u\n", address, page_number, offset);

        int tlb_hit = 0;
        int page_fault = 0;
        unsigned int frame_number = 0;

        // === TLB Lookup ===
        for (int i = 0; i < TLB_SIZE; i++) {
            if (tlb[i].valid && tlb[i].page_number == page_number) {
                tlb_hit = 1;
                frame_number = tlb[i].frame_number;
                tlb[i].last_used_time = time_counter; // Update usage time for LRU
                break;
            }
        }

        if (!tlb_hit) {
            // === Page Table Lookup ===
            if (page_table[page_number].present) {
                frame_number = page_table[page_number].frame_number;
            } else {
                page_fault = 1;

                if (next_free_frame < MAX_FRAMES) {
                    // Free frame available
                    frame_number = next_free_frame;
                    page_table[page_number].present = 1;
                    page_table[page_number].frame_number = frame_number;
                    page_order[tail] = page_number;
                    tail = (tail + 1) % MAX_FRAMES;
                    next_free_frame++;
                } else {
                    // No free frames, perform FIFO page replacement
                    unsigned int page_to_evict = page_order[head];
                    head = (head + 1) % MAX_FRAMES;
                    unsigned int evicted_frame = page_table[page_to_evict].frame_number;
                    printf("evicted-page=%u,freed-frame=%u\n", page_to_evict, evicted_frame);

                    page_table[page_to_evict].present = 0; // mark evicted page as not present

                    // --- TLB flush: Remove evicted page from TLB if present ---
                    for (int i = 0; i < TLB_SIZE; i++) {
                        if (tlb[i].valid && tlb[i].page_number == page_to_evict) {
                            tlb[i].valid = 0; // Invalidate TLB entry
                            break;
                        }
                    }

                    // Load new page into freed frame
                    frame_number = evicted_frame;
                    page_table[page_number].present = 1;
                    page_table[page_number].frame_number = frame_number;
                    page_order[tail] = page_number;
                    tail = (tail + 1) % MAX_FRAMES;
                }
            }

            // === TLB Update: Insert new page-frame mapping ===
            int empty_index = -1;
            int lru_index = 0;
            unsigned long lru_time = __LONG_MAX__; // Initialize to max value

            for (int i = 0; i < TLB_SIZE; i++) {
                if (!tlb[i].valid && empty_index == -1) {
                    empty_index = i;
                } else if (tlb[i].valid && tlb[i].last_used_time < lru_time) {
                    lru_time = tlb[i].last_used_time;
                    lru_index = i;
                }
            }

            int index_to_replace = (empty_index != -1) ? empty_index : lru_index;

            tlb[index_to_replace].valid = 1;
            tlb[index_to_replace].page_number = page_number;
            tlb[index_to_replace].frame_number = frame_number;
            tlb[index_to_replace].last_used_time = time_counter;
        }

        // === Physical Address ===
        unsigned int physical_address = (frame_number << OFFSET_BITS) | offset;

        // === Correct Output Section ===

        // Step 1: TLB Lookup Result
        if (tlb_hit) {
            printf("tlb-hit=1,page-number=%u,frame=%u,physical-address=%u\n", page_number, frame_number, physical_address);
        } else {
            printf("tlb-hit=0,page-number=%u,frame=none,physical-address=none\n", page_number);
        }

        // Step 2: TLB Remove/Add
        if (!tlb_hit) {
            printf("tlb-remove=none,tlb-add=%u\n", page_number);
        }

        // Step 3: Page Table and Page Fault Result
        if (!tlb_hit) {  // Only print this if TLB was a miss
            if (page_fault) {
                printf("page-number=%u,page-fault=1,frame-number=%u,physical-address=%u\n", page_number, frame_number, physical_address);
            } else {
                printf("page-number=%u,page-fault=0,frame-number=%u,physical-address=%u\n", page_number, frame_number, physical_address);
            }
        }
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
