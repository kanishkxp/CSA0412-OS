/* boundary_test.c - addresses at the start and end of a page and beyond the last page.
 * Uses the same page table as page_translate.c (1 KB pages, 8 pages). */
#include <stdio.h>
#define PAGE_SIZE 1024
#define PAGES 8
int page_table[PAGES] = {5, 2, -1, 7, 0, -1, 3, -1};

static void translate(unsigned a, const char *note)
{
    unsigned p = a / PAGE_SIZE, d = a % PAGE_SIZE;
    printf("%5u (%-26s) -> page %u offset %4u : ", a, note, p, d);
    if (p >= PAGES) puts("invalid address (beyond the last page - trap to OS)");
    else if (page_table[p] == -1) puts("page fault (valid page, not in memory)");
    else printf("physical %u\n", (unsigned)page_table[p] * PAGE_SIZE + d);
}
int main(void)
{
    translate(0,    "first byte of page 0");
    translate(1023, "last byte of page 0");
    translate(1024, "first byte of page 1");
    translate(3072, "first byte of page 3");
    translate(4095, "last byte of page 3");
    translate(8191, "last byte of last page 7");
    translate(8192, "first byte after last page");
    translate(100000, "far beyond the table");
    return 0;
}
