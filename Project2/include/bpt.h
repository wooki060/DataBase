#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

typedef uint64_t pagenum_t;

// Output and utility.
void usage(void);//
void print_tree(void);//
int open_table(char *pathname);//
int cut(int length);//
int db_find(int64_t key, char* ret_val);//
pagenum_t find_leaf_page(int64_t key); //
 
// Insertion.
int db_insert(int64_t key, char* value); //
void insert_into_leaf(pagenum_t leaf_page_number, int64_t key, char* value); //
void insert_into_leaf_after_splitting(pagenum_t leaf_page_number, int64_t key, char* value); //
void insert_into_parent(pagenum_t left_page_number, int64_t new_key, pagenum_t right_page_number);//
void insert_into_internal(pagenum_t page_number, int left_idx, int64_t new_key, pagenum_t last_page_number);//
void insert_into_internal_after_splitting(pagenum_t left_page_number, int left_idx, int64_t new_key, pagenum_t last_page_number);//
void insert_into_new_root(pagenum_t left_page_number, int64_t new_key, pagenum_t right_page_number); //
void start_new_tree(int64_t key, char* value);//

// Deletion.
int db_delete(int64_t key); //
void remove_entry_from_page(int64_t key, pagenum_t delete_page_number); //
void delete_entry(int64_t key, pagenum_t delete_page_number); //
void redistribute_pages(pagenum_t delete_page_number, int neighbor_page_idx, pagenum_t neighbor_page_number, pagenum_t parent_page_number, int64_t k_prime, int k_prime_idx); //
void coalesce_pages(pagenum_t delete_page_number, int neighbor_page_idx, pagenum_t neighbor_page_number, pagenum_t parent_page_number, int64_t k_prime);
void adjust_root(pagenum_t delete_page_number);
