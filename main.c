#include <stdio.h>
#include <stdint.h>

#define OFFSET_MASK 0xFFF
#define PAGE_NUMBER_MASK 0x3FF

void parse_logical_address(uint32_t logical_address);

int main(){
    uint32_t addr1 = 30023, addr2 = 10001, addr3 = 20007;
    parse_logical_address(addr1);
    parse_logical_address(addr2);
    parse_logical_address(addr3);
    return 0;
}