#include <iostream>
#include <ctype.h>
#include <string.h>
#include <fstream>
#include <list>
// #include <bits/stdc++.h>
#include <stdlib.h>
#include <unistd.h>

#include "process.h"


using namespace std;

// ------------------------------
// Global variables
// ------------------------------
int ran_count;
vector <int> randvals;

// typedef struct { … } pte_t; // can only be total of 32-bit size and will check on this 
// typedef struct { … } frame_t;
// frame_t frame_table[MAX_FRAMES];
// pte_t page_table[MAX_VPAGES]; // a per process array of fixed size=64 of pte_t not pte_t pointers !

// class Pager {
//     virtual frame_t* select_victim_frame() = 0; // virtual base class
// };

// frame_t *get_frame() {
//     frame_t *frame = allocate_frame_from_free_list();
//     if (frame == NULL) frame = THE_PAGER->select_victim_frame();
//         return frame;
// }

// while (get_next_instruction(&operation, &vpage)) {
//     // handle special case of “c” and “e” instruction
//     // now the real instructions for read and write
//     pte_t *pte = &current_process->page_table[vpage];
//     if ( ! pte->present) {
//         // this in reality generates the page fault exception and now you execute // verify this is actually a valid page in a vma if not raise error and next inst
//         frame_t *newframe = get_frame();
//         //-> figure out if/what to do with old frame if it was mapped
//         // see general outline in MM-slides under Lab3 header and writeup below
//         // see whether and how to bring in the content of the access page.
//     } // check write protection
//     // simulate instruction execution by hardware by updating the R/M PTE bits
//     update_pte(read/modify) bits based on operations.
// }


int main(int argc, char** argv) {
    int c, x;
    char *num_frames, *algo, *options;
    string rfile, infile, s_temp;
    ifstream myfile;
    while((c = getopt(argc, argv, "f:a:o:")) != -1) {
        switch(c) {
            case 'f':
                num_frames = optarg;
                break;
            case 'a':
                algo = optarg;
                break;
            case 'o':
                options = optarg;
                break;
            case '?':
                if (optopt == 'f' || optopt == 'a' || optopt == 'o')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint (optopt))
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
                return 1;
    
            default:
                abort ();
        }
    }
    rfile = argv[argc-2];
    infile = argv[argc-1];
    
    myfile.open(rfile);
    getline(myfile, s_temp);
    sscanf(&s_temp[0], "%d", &ran_count);
    while(getline(myfile, s_temp)) {
        sscanf(&s_temp[0], "%d", &x);
        randvals.push_back(x);
    }

    // for(int i=0; i<ran_count; i++) {
    //     cout<<randvals[i]<<endl;
    // }
    // string s_temp;
    // ifstream myfile;
    // for(int i=1; i<argc; i++) {
    //     if(strcmp(argv[i], "-v") == 0) v = 1;
    //     else if(strcmp(argv[i], "-t") == 0) t = 1;
    //     else if(strcmp(argv[i],"-e") == 0) e = 1;
    //     else if (argv[i][0] == '-' && argv[i][1] == 's') {
            
    //     }
    // }

    // myfile.open(argv[argc-1]);
    // getline(myfile,s_temp);
    // ran_count = stoi(s_temp);
    // while(getline(myfile, s_temp)) {
    //     randvals.push_back(stoi(s_temp));
    // }
    // myfile.close();
}