#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LEAF_SIZE 31
#define MAX_INTERNAL_SIZE 248
#define PAGE_SIZE 4096
#define VALUE_SIZE 120
typedef uint64_t pagenum_t;
typedef struct leaf_record{
    int64_t key;
    char value[120];
}leaf_record;
typedef struct internal_record{
    int64_t key;
    pagenum_t page_number;
}internal_record;
typedef struct page_t{
    union{
        struct{
            pagenum_t free_page_number;
            pagenum_t root_page_number;
            int64_t num_of_pages;
            char h_reserved[PAGE_SIZE - 8*3];
        };
        struct{
            pagenum_t next_free_or_parent_page_number;
            int32_t is_leaf;
            int32_t number_of_keys;
            char reserved[120 - (8+4*2)];
            pagenum_t right_sibling_or_one_more_page_number;
            union{
                leaf_record l_records[MAX_LEAF_SIZE-1];
                internal_record i_records[MAX_INTERNAL_SIZE-1];
            };
        };
    };
}page_t;
pagenum_t file_alloc_page();
void file_free_page(pagenum_t pagenum);
void file_read_page(pagenum_t pagenum, page_t* dest);
void file_write_page(pagenum_t pagenum, const page_t* src);
