#include "bpt.h"
#include "file.h"
int fd;
page_t* header;
page_t* root;
void usage(void){
    printf("Enter any of the following commands after the prompt > :\n"
    "\ti <key> <value>  -- Insert <key> (an integer) and <value>.\n"
    "\tf <k>  -- Find the value under key <k>.\n"
    "\td <k>  -- Delete key <k> and its associated value.\n"
    "\tt -- Print the B+ tree.\n"
    "\tq -- Quit. (Or use Ctl-D.)\n");
}
void print_tree(void){
    printf("root page_number : %llu\n",header->root_page_number);
    printf("page_num : %lld\n",header->num_of_pages);
    printf("free_page_num : %lld\n",header->free_page_number);
    if(header->root_page_number == 0){
        printf("Empty root\n");
        return;
    }
    pagenum_t q[10000];
    bool lv[10000];
    q[0] = header->root_page_number;
    lv[0] = 0;
    int i=0,j=1;
    bool check=false;
    while(i < j){
        page_t* page = (page_t*)malloc(sizeof(page_t));
        file_read_page(q[i], page);
        
        if(i>0 && lv[i-1] != lv[i]){
            printf("\n\n\n");
        }
        if(page->is_leaf){
            if(!check && i!=0){
                printf("\n\n\n");
                check = true;
            }
            for(int idx=0;idx<page->number_of_keys;idx++)
                printf("%lld ",page->l_records[idx].key);
            i++;
            printf("| ");
            free(page);
            continue;
        }
        
        q[j] = page->right_sibling_or_one_more_page_number;
        lv[j++] = lv[i]+1;
        for(int idx=0;idx<page->number_of_keys;idx++){
            q[j] = page->i_records[idx].page_number;
            lv[j++] = lv[i]+1;
            printf("%lld ",page->i_records[idx].key);
        }
        printf("| ");
        free(page);
        i++;
    }
    printf("\n");
}
int open_table(char* pathname){
    header = (page_t*)malloc(sizeof(page_t));
    fd = open(pathname, O_RDWR | O_SYNC);
    if(fd == -1){
        //만약 존재하지 않는다면 새로운 파일을 생성한다.
        fd = open(pathname, O_RDWR | O_SYNC | O_CREAT, S_IRWXU);
        if(fd ==-1){
            fprintf(stderr,"cannot open table %s\n",strerror(errno));
            return -1;
        }
        header->free_page_number = 0;
        header->root_page_number = 0;
        header->num_of_pages = 1;
        file_write_page(0,header);
    }
    else{
        root = (page_t*)malloc(sizeof(page_t));
        file_read_page(0,header);
        pagenum_t root_page_number = header->root_page_number;
        file_read_page(root_page_number, root);
    }
    return fd;
}
int cut(int length){
    return (length+1)/2;
}
int db_find(int64_t key, char* ret_val){
    pagenum_t page_number = find_leaf_page(key);
    if(page_number == -1)
        return -1;
    
    page_t* p = (page_t*)malloc(sizeof(page_t));
    file_read_page(page_number,p);
    
    for(int i=0; i < p->number_of_keys; i++){
        if(p->l_records[i].key == key){
            strcpy(ret_val, p->l_records[i].value);
            free(p);
            return 0;
        }
    }
    free(p);
    return -1;
}
pagenum_t find_leaf_page(int64_t key){
    pagenum_t page_number = header->root_page_number;
    page_t* page = (page_t*)malloc(sizeof(page_t));
    file_read_page(page_number, page);
    
    while(!page->is_leaf){
        if(key < page->i_records[0].key)
            page_number = page->right_sibling_or_one_more_page_number;
        else{
            int i=1;
            while(i<page->number_of_keys && key >= page->i_records[i].key){
                i++;
            }
            page_number = page->i_records[i-1].page_number;
        }
        file_read_page(page_number,page);
    }
    free(page);
    return page_number;
}

// Insertion.
int db_insert(int64_t key, char* value){
    if(header->root_page_number == 0){
        start_new_tree(key, value);
        return 0;
    }
    //existed key
    char *tmp;
    tmp = (char*)malloc(sizeof(char)*VALUE_SIZE);
    if(db_find(key, tmp) == 0)
        return -1;
    free(tmp);
    
    page_t* leaf = (page_t*)malloc(sizeof(page_t));
    pagenum_t leaf_page_number = find_leaf_page(key);
    file_read_page(leaf_page_number, leaf);
    
    //add record
    if(leaf->number_of_keys < MAX_LEAF_SIZE){
        free(leaf);
        insert_into_leaf(leaf_page_number, key, value);
    }
    //need splitting
    else{
        free(leaf);
        insert_into_leaf_after_splitting(leaf_page_number, key, value);
    }
    return 0;
}
void insert_into_leaf(pagenum_t leaf_page_number, int64_t key, char* value){
    page_t* leaf = (page_t*)malloc(sizeof(page_t));
    file_read_page(leaf_page_number, leaf);
    
    //find insrtion_idx;
    int insertion_idx=0;
    while(insertion_idx < leaf->number_of_keys && leaf->l_records[insertion_idx].key < key)
        insertion_idx++;
    for(int i=leaf->number_of_keys; i>insertion_idx; i--)
        leaf->l_records[i] = leaf->l_records[i-1];
    leaf->l_records[insertion_idx].key = key;
    strcpy(leaf->l_records[insertion_idx].value, value);
    leaf->number_of_keys++;
    file_write_page(leaf_page_number, leaf);
    free(leaf);
}
void insert_into_leaf_after_splitting(pagenum_t leaf_page_number, int64_t key, char* value){
    page_t* new_leaf = (page_t*)malloc(sizeof(page_t));
    pagenum_t new_leaf_page_number;
    new_leaf_page_number = file_alloc_page();
    file_read_page(new_leaf_page_number,new_leaf);
    new_leaf->is_leaf=1;
    page_t* leaf = (page_t*)malloc(sizeof(page_t));
    file_read_page(leaf_page_number, leaf);
    
    //find insertion_idx;
    int insertion_idx=0;
    while(insertion_idx < MAX_LEAF_SIZE && leaf->l_records[insertion_idx].key < key)
        insertion_idx++;
    
    //copy record
    leaf_record* tmp_record = (leaf_record*)malloc(sizeof(leaf_record)*(MAX_LEAF_SIZE+1));
    for(int i=0, j=0; i<leaf->number_of_keys; i++, j++){
        if(j==insertion_idx)
            j++;
        tmp_record[j].key = leaf->l_records[i].key;
        strcpy(tmp_record[j].value, leaf->l_records[i].value);
    }
    tmp_record[insertion_idx].key = key;
    strcpy(tmp_record[insertion_idx].value, value);
    
    //0~split -> origin page
    int split = cut(MAX_LEAF_SIZE+1);
    leaf->number_of_keys=split;
    for(int i=0; i<split; i++){
        leaf->l_records[i].key = tmp_record[i].key;
        strcpy(leaf->l_records[i].value, tmp_record[i].value);
    }
    //split ~ MAX_LEAF_SIZE -> new page
    new_leaf->number_of_keys = MAX_LEAF_SIZE+1 - split;
    for(int i=split; i<MAX_LEAF_SIZE+1; i++){
        new_leaf->l_records[i-split].key = tmp_record[i].key;
        strcpy(new_leaf->l_records[i-split].value, tmp_record[i].value);
    }
    //Page Header setting
    new_leaf->right_sibling_or_one_more_page_number = leaf->right_sibling_or_one_more_page_number;
    leaf->right_sibling_or_one_more_page_number = new_leaf_page_number;
    new_leaf->next_free_or_parent_page_number = leaf->next_free_or_parent_page_number;
    int64_t new_key = new_leaf->l_records[0].key;
    
    file_write_page(leaf_page_number, leaf);
    file_write_page(new_leaf_page_number, new_leaf);
    free(leaf);
    free(new_leaf);
    insert_into_parent(leaf_page_number, new_key, new_leaf_page_number);
}
void insert_into_parent(pagenum_t left_page_number, int64_t new_key, pagenum_t right_page_number){
    //find parnet page
    page_t* left = (page_t*)malloc(sizeof(page_t));
    file_read_page(left_page_number, left);
    pagenum_t parent_page_number = left->next_free_or_parent_page_number;
    
    free(left);
    //parent == header
    if(parent_page_number==0){
        insert_into_new_root(left_page_number, new_key, right_page_number);
        return;
    }
    page_t* parent = (page_t*)malloc(sizeof(page_t));
    file_read_page(parent_page_number, parent);
    
    //find added idx
    int left_idx;
    if(left_page_number == parent->right_sibling_or_one_more_page_number)
        left_idx = -1;
    else{
        left_idx=0;
        while(left_idx < parent->number_of_keys && parent->i_records[left_idx].page_number != left_page_number){left_idx++;}
        if(left_idx == parent->number_of_keys){
            left_idx = -1;
        }
    }
    
    //add record
    if(parent->number_of_keys < MAX_INTERNAL_SIZE){
        insert_into_internal(parent_page_number, left_idx, new_key, right_page_number);
        free(parent);
    }
    //need splitting
    else{
        free(parent);
        insert_into_internal_after_splitting(parent_page_number, left_idx, new_key, right_page_number);
    }
}
void insert_into_internal(pagenum_t page_number, int left_idx, int64_t new_key, pagenum_t last_page_number){
    page_t* page = (page_t*)malloc(sizeof(page_t));
    file_read_page(page_number, page);
    for(int i=page->number_of_keys; i>left_idx+1; i--){
        page->i_records[i].page_number = page->i_records[i-1].page_number;
        page->i_records[i].key = page->i_records[i-1].key;
    }
    page->i_records[left_idx+1].page_number = last_page_number;
    page->i_records[left_idx+1].key = new_key;
    page->number_of_keys++;
    file_write_page(page_number, page);
    free(page);
    if(page_number == header->root_page_number){
        file_read_page(page_number, root);
        file_write_page(page_number, root);
    }
}
void insert_into_internal_after_splitting(pagenum_t left_page_number, int left_idx, int64_t new_key, pagenum_t last_page_number){
    page_t* left = (page_t*)malloc(sizeof(page_t));
    file_read_page(left_page_number, left);
    
    page_t* right = (page_t*)malloc(sizeof(page_t));
    pagenum_t right_page_number = file_alloc_page();
    file_read_page(right_page_number, right);
    internal_record* tmp_record = (internal_record*)malloc(sizeof(internal_record)*(MAX_INTERNAL_SIZE+1));
    for(int i=0, j=0; i<left->number_of_keys; i++, j++){
        if(j==left_idx+1)
            j++;
        tmp_record[j].key = left->i_records[i].key;
        tmp_record[j].page_number = left->i_records[i].page_number;
    }
    tmp_record[left_idx+1].key = new_key;
    tmp_record[left_idx+1].page_number = last_page_number;
    
    //left page settiing
    int split = cut(MAX_INTERNAL_SIZE+1);
    left->number_of_keys=split-1;
    for(int i=0; i<split-1; i++){
        left->i_records[i].key = tmp_record[i].key;
        left->i_records[i].page_number = tmp_record[i].page_number;
    }
    
    //add parent key
    int64_t k_prime = tmp_record[split-1].key;
    
    //right page setting
    right->right_sibling_or_one_more_page_number = tmp_record[split-1].page_number;
    right->number_of_keys = MAX_INTERNAL_SIZE+1 - split;
    for(int i = split; i<MAX_INTERNAL_SIZE+1; i++){
        right->i_records[i-split].key = tmp_record[i].key;
        right->i_records[i-split].page_number = tmp_record[i].page_number;
    }
    free(tmp_record);
    right->next_free_or_parent_page_number = left->next_free_or_parent_page_number;
    
    //child page setting
    page_t* child_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(right->right_sibling_or_one_more_page_number, child_page);
    child_page->next_free_or_parent_page_number = right_page_number;
    file_write_page(right->right_sibling_or_one_more_page_number, child_page);
    for(int i=0; i<right->number_of_keys; i++){
        int64_t child_page_number = right->i_records[i].page_number;
        file_read_page(child_page_number, child_page);
        child_page->next_free_or_parent_page_number = right_page_number;
        file_write_page(child_page_number,child_page);
    }
    
    file_write_page(right_page_number, right);
    file_write_page(left_page_number, left);
    free(right);
    free(left);
    //parent setting
    insert_into_parent(left_page_number, k_prime, right_page_number);
}
void insert_into_new_root(pagenum_t left_page_number, int64_t new_key, pagenum_t right_page_number){
    page_t* new_root = (page_t*)malloc(sizeof(page_t));
    pagenum_t new_root_page_number = file_alloc_page();
    file_read_page(new_root_page_number, new_root);
    new_root->is_leaf=0;
    new_root->i_records[0].key = new_key;
    new_root->i_records[0].page_number = right_page_number;
    new_root->right_sibling_or_one_more_page_number = left_page_number;
    new_root->number_of_keys++;
    new_root->next_free_or_parent_page_number=0;
    
    page_t* left_page = (page_t*)malloc(sizeof(page_t));
    page_t* right_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(left_page_number, left_page);
    file_read_page(right_page_number, right_page);
    
    left_page->next_free_or_parent_page_number = new_root_page_number;
    right_page->next_free_or_parent_page_number = new_root_page_number;
    file_write_page(left_page_number, left_page);
    file_write_page(right_page_number, right_page);
    file_write_page(new_root_page_number, new_root);
    header->root_page_number = new_root_page_number;
    file_write_page(0,header);
    file_read_page(header->root_page_number, root);
    file_read_page(0,header);
}
void start_new_tree(int64_t key, char* value){
    root = (page_t*)malloc(sizeof(page_t));
    pagenum_t root_page_number = file_alloc_page();
    header->root_page_number = root_page_number;
    file_read_page(root_page_number, root);
    root->is_leaf=1;
    root->l_records[0].key = key;
    strcpy(root->l_records[0].value, value);
    root->number_of_keys = 1;
    file_write_page(root_page_number, root);
    file_write_page(0,header);
}

// Deletion.
int db_delete(int64_t key){
    //not root page or empty root
    if(header->root_page_number==0 || root->number_of_keys==0)
        return -1;
    
    //not exist key
    char *tmp;
    tmp = (char*)malloc(sizeof(char)*VALUE_SIZE);
    if(db_find(key, tmp) == -1)
        return -1;
    
    pagenum_t delete_page_number = find_leaf_page(key);
    delete_entry(key, delete_page_number);
    return 0;
}
void delete_entry(int64_t key, pagenum_t delete_page_number){
    remove_entry_from_page(key, delete_page_number);
    if(delete_page_number == header->root_page_number){
        adjust_root(delete_page_number);
        return;
    }
    
    page_t* delete_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(delete_page_number, delete_page);
    //delay merge
    if(delete_page->number_of_keys > 0){
        free(delete_page);
        return;
    }
    int max_page;
    if(delete_page->is_leaf)
        max_page = MAX_LEAF_SIZE;
    else
        max_page = MAX_INTERNAL_SIZE;
    
    pagenum_t parent_page_number = delete_page->next_free_or_parent_page_number;
    page_t* parent_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(parent_page_number, parent_page);
    
    pagenum_t neighbor_page_number;
    int k_prime_idx;
    int64_t k_prime;
    int neighbor_page_idx;
    if(delete_page_number == parent_page->right_sibling_or_one_more_page_number){
        neighbor_page_idx = -2;
        neighbor_page_number = parent_page->i_records[0].page_number;
        k_prime_idx = 0;
        k_prime = parent_page->i_records[0].key;
    }
    else if(delete_page_number == parent_page->i_records[0].page_number){
        neighbor_page_idx = -1;
        neighbor_page_number = parent_page->right_sibling_or_one_more_page_number;
        k_prime_idx = 0;
        k_prime = parent_page->i_records[0].key;
    }
    else{
        int i=0;
        while(i<parent_page->number_of_keys && delete_page_number != parent_page->i_records[i].page_number){i++;}
        neighbor_page_idx = i-1;
        neighbor_page_number = parent_page->i_records[neighbor_page_idx].page_number;
        k_prime_idx = neighbor_page_idx+1;
        k_prime = parent_page->i_records[k_prime_idx].key;
    }
    page_t* neighbor_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(neighbor_page_number, neighbor_page);
    
    if(neighbor_page->number_of_keys + delete_page->number_of_keys > max_page)
        redistribute_pages(delete_page_number, neighbor_page_idx, neighbor_page_number, parent_page_number, k_prime, k_prime_idx);
    else
        coalesce_pages(delete_page_number, neighbor_page_idx, neighbor_page_number, parent_page_number, k_prime);
}
void remove_entry_from_page(int64_t key, pagenum_t delete_page_number){
    int i;
    page_t* delete_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(delete_page_number, delete_page);
    //internal page
    if(!delete_page->is_leaf){
        i=0;
        while(delete_page->i_records[i].key != key)
            i++;
        for(++i; i<delete_page->number_of_keys; i++){
            delete_page->i_records[i-1].key = delete_page->i_records[i].key;
            delete_page->i_records[i-1].page_number = delete_page->i_records[i].page_number;
        }
        delete_page->i_records[i-1].key = 0;
        delete_page->i_records[i-1].page_number = 0;
        delete_page->number_of_keys--;
        file_write_page(delete_page_number, delete_page);
        if(delete_page_number == header->root_page_number)
            file_read_page(delete_page_number, root);
    }
    //leaf page
    else{
        i=0;
        while(delete_page->l_records[i].key != key)
            i++;
        for(++i; i < delete_page->number_of_keys; i++){
            delete_page->l_records[i-1].key = delete_page->l_records[i].key;
            strcpy(delete_page->l_records[i-1].value , delete_page->l_records[i].value);
        }
        delete_page->l_records[i-1].key = 0;
        delete_page->number_of_keys--;
        file_write_page(delete_page_number, delete_page);
        if(delete_page_number == header->root_page_number)
            file_read_page(delete_page_number, root);
    }
}
void redistribute_pages(pagenum_t delete_page_number, int neighbor_page_idx, pagenum_t neighbor_page_number, pagenum_t parent_page_number, int64_t k_prime, int k_prime_idx){
    page_t* delete_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(delete_page_number, delete_page);
    page_t* neighbor_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(neighbor_page_number, neighbor_page);
    page_t* parent_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(parent_page_number, parent_page);
    if(neighbor_page_idx == -2){
        if(delete_page->is_leaf){
            delete_page->l_records[delete_page->number_of_keys].key = neighbor_page->l_records[0].key;
            strcpy(delete_page->l_records[delete_page->number_of_keys].value, neighbor_page->l_records[0].value);
            delete_page->number_of_keys++;
            int i;
            for(i=0; i<neighbor_page->number_of_keys-1; i++){
                neighbor_page->l_records[i].key = neighbor_page->l_records[i+1].key;
                strcpy(neighbor_page->l_records[i].value, neighbor_page->l_records[i+1].value);
            }
            neighbor_page->l_records[i].key = 0;
            neighbor_page->number_of_keys--;
            parent_page->i_records[k_prime_idx].key = neighbor_page->l_records[0].key;
            parent_page->i_records[k_prime_idx].page_number = neighbor_page_number;
        }
        else{
            delete_page->i_records[delete_page->number_of_keys].key = k_prime;
            delete_page->i_records[delete_page->number_of_keys].page_number = neighbor_page->right_sibling_or_one_more_page_number;
            page_t* child_page = (page_t*)malloc(sizeof(page_t));
            file_read_page(delete_page->i_records[delete_page->number_of_keys].page_number, child_page);
            child_page->next_free_or_parent_page_number = delete_page_number;
            file_write_page(delete_page->i_records[delete_page->number_of_keys].page_number, child_page);
            delete_page->number_of_keys++;
            neighbor_page->right_sibling_or_one_more_page_number = neighbor_page->i_records[0].page_number;

            int i;
            for(i=0; i<neighbor_page->number_of_keys-1; i++){
                neighbor_page->i_records[i].key = neighbor_page->i_records[i+1].key;
                neighbor_page->i_records[i].page_number = neighbor_page->i_records[i+1].page_number;
            }
            neighbor_page->i_records[i].key=0;
            neighbor_page->i_records[i].page_number=0;
            neighbor_page->number_of_keys--;
            parent_page->i_records[k_prime_idx].key = neighbor_page->i_records[0].key;
        }
    }
    else{
        if(delete_page->is_leaf){
            int i;
            for(i=delete_page->number_of_keys; i>0; i--){
                delete_page->l_records[i].key = delete_page->l_records[i-1].key;
                strcpy(delete_page->l_records[i].value, delete_page->l_records[i-1].value);
            }
            delete_page->l_records[0].key = neighbor_page->l_records[neighbor_page->number_of_keys-1].key;
            strcpy(delete_page->l_records[0].value, delete_page->l_records[neighbor_page->number_of_keys-1].value);
            neighbor_page->l_records[neighbor_page->number_of_keys-1].key=0;
            parent_page->i_records[k_prime_idx].key = delete_page->l_records[0].key;
            neighbor_page->number_of_keys--;
            delete_page->number_of_keys++;
        }
        else{
            int i;
            for(i=delete_page->number_of_keys; i>0; i--){
                delete_page->i_records[i].key = delete_page->i_records[i-1].key;
                delete_page->i_records[i].page_number = delete_page->i_records[i-1].page_number;
            }
            delete_page->i_records[0].key = k_prime;
            delete_page->i_records[0].page_number = delete_page->right_sibling_or_one_more_page_number;
            delete_page->right_sibling_or_one_more_page_number = neighbor_page->i_records[neighbor_page->number_of_keys-1].page_number;
            neighbor_page->number_of_keys--;
            delete_page->number_of_keys++;
            page_t* child_page = (page_t*)malloc(sizeof(page_t));
            file_read_page(delete_page->right_sibling_or_one_more_page_number, child_page);
            child_page->next_free_or_parent_page_number = delete_page_number;
            file_write_page(delete_page->right_sibling_or_one_more_page_number,child_page);
            parent_page->i_records[k_prime_idx].key = delete_page->i_records[0].key;
        }
    }
    file_write_page(delete_page_number, delete_page);
    file_write_page(parent_page_number, parent_page);
    file_write_page(neighbor_page_number, neighbor_page);
}
void coalesce_pages(pagenum_t delete_page_number, int neighbor_page_idx, pagenum_t neighbor_page_number, pagenum_t parent_page_number, int64_t k_prime){
    int i, j, left_insertion_idx, left_end;
    if(neighbor_page_idx == -2){
        pagenum_t tmp_page_number;
        tmp_page_number = delete_page_number;
        delete_page_number = neighbor_page_number;
        neighbor_page_number = tmp_page_number;
    }
    page_t* delete_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(delete_page_number, delete_page);
    page_t* neighbor_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(neighbor_page_number, neighbor_page);
    page_t* parent_page = (page_t*)malloc(sizeof(page_t));
    file_read_page(parent_page_number, parent_page);
    
    left_insertion_idx = neighbor_page->number_of_keys;
    left_end = delete_page->number_of_keys;
    if(delete_page->is_leaf){
        for(i=left_insertion_idx, j=0; j<left_end; i++, j++){
            neighbor_page->l_records[i].key = delete_page->l_records[j].key;
            strcpy(neighbor_page->l_records[i].value, delete_page->l_records[j].value);
            neighbor_page->number_of_keys++;
            delete_page->number_of_keys--;
        }
        neighbor_page->right_sibling_or_one_more_page_number = delete_page->right_sibling_or_one_more_page_number;
    }
    else{
        neighbor_page->i_records[left_insertion_idx].key = k_prime;
        neighbor_page->i_records[left_insertion_idx].page_number = delete_page->right_sibling_or_one_more_page_number;
        neighbor_page->number_of_keys++;
        
        for(i=left_insertion_idx+1, j=0; j<left_end; i++, j++){
            neighbor_page->i_records[i].key = delete_page->i_records[j].key;
            neighbor_page->i_records[i].page_number = delete_page->i_records[j].page_number;
            neighbor_page->number_of_keys++;
            delete_page->number_of_keys--;
        }
        for(i=0; i<neighbor_page->number_of_keys+1; i++){
            page_t* tmp = (page_t*)malloc(sizeof(page_t));
            file_read_page(neighbor_page->i_records[i].page_number, tmp);
            tmp->next_free_or_parent_page_number = neighbor_page_number;
            free(tmp);
        }
    }
    file_write_page(neighbor_page_number, neighbor_page);
    delete_entry(k_prime, parent_page_number);
    file_free_page(delete_page_number);
}
void adjust_root(pagenum_t delete_page_number){
    if(root->number_of_keys==0){
        if(!root->is_leaf){
            pagenum_t new_root_page_number = root->right_sibling_or_one_more_page_number;
            page_t* new_root = (page_t*)malloc(sizeof(page_t));
            file_read_page(new_root_page_number, new_root);
            new_root->next_free_or_parent_page_number=0;
            file_free_page(delete_page_number);
            header->root_page_number = new_root_page_number;
            file_write_page(0, header);
            file_write_page(new_root_page_number, new_root);
            file_read_page(0,header);
            file_read_page(header->root_page_number, root);
        }
        else{
            file_free_page(delete_page_number);
            header->root_page_number = 0;
            file_write_page(0,header);
            file_read_page(0, header);
        }
    }
}
