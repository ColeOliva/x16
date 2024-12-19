#include <stdio.h>
#include <string.h>
#include "translation.h"

int main() {
    TLB TheTlb;
    memset(&TheTlb, 0, sizeof(TLB));

    // Create a TLB with some values
    TheTlb.entries[0].valid = 1;
    TheTlb.entries[0].order = 3;
    TheTlb.entries[0].virtual_page_no = 0xb;
    TheTlb.entries[0].physical_page_no = 0x1;

    TheTlb.entries[1].valid = 0;
    TheTlb.entries[1].order = 1;
    TheTlb.entries[1].virtual_page_no = 0xa;
    TheTlb.entries[1].physical_page_no = 0x4;

    TheTlb.entries[2].valid = 1;
    TheTlb.entries[2].order = 2;
    TheTlb.entries[2].virtual_page_no = 0x6;
    TheTlb.entries[2].physical_page_no = 0x2;

    TheTlb.entries[3].valid = 1;
    TheTlb.entries[3].order = 0;
    TheTlb.entries[3].virtual_page_no = 0x7;
    TheTlb.entries[3].physical_page_no = 0x3;

    // Try looking up a value
    unsigned short virtual_address = 0x6abc;
    unsigned short physical_address = 0;
    bool result = tlb_lookup(&TheTlb, virtual_address, &physical_address);
    // Should be true, with the physical address as 2abc
    printf("Result: %d: Physical Address=%x\n", result, physical_address);

    // The order should be 3, 2, 0, 1 for each entry
    tlb_dump(&TheTlb);
}
