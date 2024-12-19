#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

// A page table entry (PTE) has the
// corresponding physical page number. If the valid bit is not set,
// then the PTE is invalid
typedef struct {
    unsigned char valid: 1;
    unsigned char physical_page_no: 4;
} PageTableEntry;

// The page table has 16 entries
#define PAGE_TABLE_SIZE 16

// A page table is an array of page table entries
typedef struct {
    PageTableEntry entries[PAGE_TABLE_SIZE];
} PageTable;

#endif // PAGE_TABLE_H
