#include "bpt.h"

// MAIN

int main( int argc, char ** argv ) {
    char instruction;
    char *pathname = "sample_10000.db";
    
    if(argc > 1)
        open_table(argv[1]);
    else
        open_table(pathname);
        
    usage();
    printf("> ");
    while (scanf("%c", &instruction) != EOF) {
        int64_t key;
        char value[120];
        char *ret_val;
        int st,en;
        switch(instruction){
            case 's':
                scanf("%d%d",&st,&en);
                for(int i=st;i<en;i++){
                    db_insert(i,"test");
                }
                break;
            case 'i':
                scanf("%lld %s",&key,value);
                if(db_insert(key, value) == -1)
                    printf("existed Key\n");
                else
                    printf("insert Key: %lld  Value:%s\n",key,value);
                break;
            case 'f':
                scanf("%lld",&key);
                ret_val = (char*)malloc(sizeof(char)*120);
                if(db_find(key,ret_val) == 0)
                    printf("Key: %lld Value: %s\n",key, ret_val);
                else
                    printf("Not existed Key\n");
                free(ret_val);
                break;
            case 'd':
                scanf("%lld",&key);
                if(db_delete(key) == -1)
                    printf("Not existed\n");
                else
                    printf("Delete %lld\n",key);
                break;
            case 't':
                print_tree();
                break;
            case 'q':
                while (getchar() != (int)'\n');
                return EXIT_SUCCESS;
            default:
                usage();
        }
        while (getchar() != (int)'\n');
        printf("> ");
    }
    printf("\n");

    return EXIT_SUCCESS;
}
