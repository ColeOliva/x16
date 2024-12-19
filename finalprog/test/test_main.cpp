#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#define ASSERT_EQ(a, b)     REQUIRE((a) == (b))
#define ASSERT_TRUE(a)      REQUIRE(a)
#define ASSERT_FALSE(a)     REQUIRE(!(a))

extern "C" {
#include "translation.h"
}


TEST_CASE("Translation.ToPage", "[Translation]") {
    unsigned short virtual_address = (unsigned short) 0x3ff2;
    ASSERT_EQ(0x3, to_page(virtual_address));
}

TEST_CASE("Translation.ToOffset", "[Translation]") {
    unsigned short virtual_address = (unsigned short) 0x3ff2;
    ASSERT_EQ(0xff2, to_offset(virtual_address));
}

TEST_CASE("Translation.ToAddress", "[Translation]") {
    unsigned short page_no = 0xa;
    unsigned short offset = 0x345;
    ASSERT_EQ(0xa345, to_address(page_no, offset));
}

TLB TheTlb;

void TLB_INIT() {
    memset(&TheTlb, 0, sizeof(TLB));

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
}


TEST_CASE("Translation.LookupFound", "[Translation]") {
    TLB_INIT();
    unsigned int virtual_address = 0x6abc;
    unsigned short physical_address = 0;
    ASSERT_EQ(true, tlb_lookup(&TheTlb, virtual_address, &physical_address));
    ASSERT_EQ(0x2abc, physical_address);
}


TEST_CASE("Translation.LookupNotFound", "[Translation]") {
    TLB_INIT();
    unsigned int virtual_address = 0xdabc;
    unsigned short physical_address = 0;
    ASSERT_EQ(false, tlb_lookup(&TheTlb, virtual_address, &physical_address));
}

TEST_CASE("Translation.Order", "[Translation]") {
    TLB_INIT();
    unsigned int virtual_address = 0x6abc;
    unsigned short physical_address = 0;
    tlb_lookup(&TheTlb, virtual_address, &physical_address);

    ASSERT_EQ(3, TheTlb.entries[0].order);
    ASSERT_EQ(2, TheTlb.entries[1].order);
    ASSERT_EQ(0, TheTlb.entries[2].order);
    ASSERT_EQ(1, TheTlb.entries[3].order);
}

PageTable ThePageTable;

void PAGETABLE_INIT() {
    memset(&ThePageTable, 0, sizeof(ThePageTable));
    ThePageTable.entries[0xd].valid = 1;
    ThePageTable.entries[0xd].physical_page_no = 7;
}

TEST_CASE("Translation.Load", "[Translation]") {
    TLB_INIT();
    PAGETABLE_INIT();
    unsigned int virtual_page_no = 0xd;
    unsigned short physical_address = 0;
    TLB_Entry* entry = tlb_load(&TheTlb, virtual_page_no, &ThePageTable);
    REQUIRE(entry != NULL);
    ASSERT_EQ(0xd, entry->virtual_page_no);
    ASSERT_EQ(0x7, entry->physical_page_no);
}

TEST_CASE("Translation.LookupAndLoad", "[Translation]") {
    TLB_INIT();
    PAGETABLE_INIT();
    unsigned int virtual_address = 0xd123;
    unsigned short physical_address = 0;
    ASSERT_EQ(true,
        tlb_lookup_and_load(&TheTlb, &ThePageTable, virtual_address,
                            &physical_address));
    ASSERT_EQ(0x7123, physical_address);
}
