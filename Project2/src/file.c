#include "file.h"
extern int fd;
extern page_t* header;

pagenum_t file_alloc_page(){
    pagenum_t last_page_number;
    pagenum_t free_page_number = header->free_page_number;
    page_t* free_page = (page_t*)malloc(sizeof(page_t));
    if(free_page_number == 0){
        last_page_number = lseek(fd, 0, SEEK_END);
        if(last_page_number == -1){
            fprintf(stderr,"cannot find page_number %s\n",strerror(errno));
            return -1;
        }
        header->num_of_pages++;
        file_write_page(0,header);
        free_page->right_sibling_or_one_more_page_number = 0;
        free_page->next_free_or_parent_page_number = 0;
        free_page->is_leaf = 0;
        free_page->number_of_keys=0;
        file_write_page(last_page_number/PAGE_SIZE, free_page);
        return last_page_number/PAGE_SIZE;
    }
    file_read_page(free_page_number, free_page);
    header->free_page_number = free_page->next_free_or_parent_page_number;
    free(free_page);
    file_write_page(0,header);
    return free_page_number;
}
void file_free_page(pagenum_t pagenum){
    page_t* page = (page_t*)malloc(sizeof(page_t));
    file_read_page(pagenum, page);
    page->next_free_or_parent_page_number=header->free_page_number;
    header->free_page_number = pagenum;
    page->is_leaf=0;
    page->number_of_keys=0;
    page->right_sibling_or_one_more_page_number=0;
    file_write_page(pagenum, page);
    file_write_page(0,header);
    file_read_page(0,header);
}
void file_read_page(pagenum_t pagenum, page_t* dest){
    if(pread(fd, dest, sizeof(page_t), pagenum*PAGE_SIZE)==-1){
        fprintf(stderr,"cannot read page %s\n",strerror(errno));
        return;
    }
}
void file_write_page(pagenum_t pagenum, const page_t* src){
    if(pwrite(fd, src, PAGE_SIZE, pagenum*PAGE_SIZE)==-1){
        fprintf(stderr,"cannot write page %s\n",strerror(errno));
        return;
    }
}
