#ifndef TRANSLATION_H
#define TRANSLATION_H

#include <stdbool.h>
#include "page_table.h"

// Virtual addresses in our system are 16 bits.
// The upper 4 bits indicate the page number, so there are 16 pages.
// Each pages has 4096 bytes, which is a 12 bit offset.

// A TLB entry consists of the following fields.
// 1 bit that indicates whether the TLB entry is valid
// 2 bits indicating the last used order
// 4 bits for the virtual page number
// 4 bits for the physical page number
typedef struct {
    unsigned int valid: 1;
    unsigned short order: 2;
    unsigned short virtual_page_no: 4;
    unsigned short physical_page_no: 4;
} TLB_Entry;

// The number of TLB entries in the TLB
#define TLB_SIZE     4

// A TLB is an array of entries of TLB_SIZE
typedef struct {
    TLB_Entry entries[TLB_SIZE];   // an array of entries
} TLB;


// Given a virtual address, return the virtual page no.
// The virtual page number is the upper 4 bits of the 16 bit virtual
// address.
unsigned short to_page(unsigned short virtual_address);


// Given a virtual address, return the offset within the physical page.
// This is the lower 12 bits of the address.
unsigned short to_offset(unsigned short virtual_address);


// Create an address out of a page number and an offset
unsigned short to_address(unsigned short page_no, unsigned short offset);


// Dump the TLB to stdout. Useful for debugging.
void tlb_dump(TLB* tlb);


// Lookup a virtual address. Given a virtual address, lookup the virtual
// address in the TLB. If the virtual address is present in the TLB,
// set *physical_address and return true.
// If the TLB does not contain the virtual address, return false.
// The lookup order in the TLB is updated.
bool tlb_lookup(TLB* tlb, unsigned short virtual_address,
     unsigned short* physical_address);


// Update the last used order for the TLB Entry
void tlb_set_order(TLB* tlb, int last_used_index);


 // Evict an entry from the TLB and read in the corresponding page table
 // entry for the given virtual address from the page table. The replacement
 // policy is to evict the least recently used entry.
 TLB_Entry* tlb_load(TLB* tlb, unsigned short virtual_page_no,
                     PageTable* ptab);

// Lookup a virtual address. Given a virtual address, lookup the virtual
// address in the TLB to get the physical address. If the TLB doesn't hold the
// physical address, load it from the page table. If the page table entry
// is invalid, return false. If the page table is valid, set
// *physical_address to the physical address, and return true.
bool tlb_lookup_and_load(TLB* tlb, PageTable* ptab,
                unsigned short virtual_address,
                unsigned short* physical_address);


#endif // TRANSLATION_H
