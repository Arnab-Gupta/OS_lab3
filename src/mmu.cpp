#include <iostream>
#include <ctype.h>
#include <string.h>
#include <fstream>
#include <list>
#include <vector>
#include <queue>
#include <stdlib.h>
#include <unistd.h>
#include <deque>

#include "process.h"
#include "pager.h"

using namespace std;

// ------------------------------
// Constants
// ------------------------------
#define N_PTE_ENTRIES 64
#define MAX_FRAMES 128

// ------------------------------
// Global variables
// ------------------------------
int ran_count;
int proc_count;
int frame_count;
long long instr_ind = 0;
char operation;
int vpage;
vector <int> randvals;
vector <pair<char, int>> instructions;
ifstream myfile;
string s_temp;
list <Process*> proc_list;
Pager *THE_PAGER;
deque <frame_t*> free_list;

// ------------------------------
// Helper functions
// ------------------------------
void printVMAs();
void printInstrs();
string getLineV2();

void printVMAs() {
    Process *p;
    VMA *v;
    for(list<Process*>::iterator it = proc_list.begin(); it != proc_list.end(); ++it) {
        p = *it;
        cout<<"\nProcess #"<<p->pid<<endl;
        for(vector<VMA*>::iterator it2 = p->vma_table.begin(); it2 != p->vma_table.end(); ++it2) {
            v = *it2;
            cout<<v->start_vpage<<" "<<v->end_vpage<<" "<<v->write_protected<<" "<<v->file_mapped<<endl;
        }
    }
}

void printInstrs() {
    cout<<"\nInstructions:-\n";
    for(vector<pair<char,int>>::iterator it = instructions.begin(); it != instructions.end(); ++it) {
        cout<<it->first<<" "<<it->second<<endl;
    }
}

string getLineV2() {
    do {
        getline(myfile, s_temp);
    } while(s_temp[0] == '#' and !myfile.eof());
    return s_temp;
}

// ------------------------------
// Main functions
// ------------------------------
void generateFreeList();
frame_t* allocate_frame_from_free_list();
frame_t* get_frame();
bool get_next_instruction(char &, int &);
void Simulation();

void generateFreeList() {
    struct frame_t *fp;
    for(int i=0; i < THE_PAGER->frame_count; i++) {
        fp = &THE_PAGER->frame_table[i];
        free_list.push_back(fp);
    }
}

frame_t* allocate_frame_from_free_list() {
    frame_t *fp;
    fp = free_list.front();
    free_list.pop_front();
    return fp;
}

frame_t *get_frame() {
    frame_t *frame = allocate_frame_from_free_list();
    if (frame == NULL) frame = THE_PAGER->select_victim_frame();
        return frame;
}

bool get_next_instruction(char &op, int &vp) {
    string intxt;
    intxt = getLineV2();
    // cout<<intxt<<endl;
    if (intxt[0] != '#' and !myfile.eof()) {
        sscanf(&intxt[0], "%c %d", &op, &vp);
        return true;
    }
    return false;
}

void Simulation() {
    while (get_next_instruction(operation, vpage)) {
        // cout<<"op: "<<operation<<"\tvp: "<<vpage<<endl;
        // handle special case of “c” and “e” instruction
        // now the real instructions for read and write
        // pte_t *pte = &current_process->page_table[vpage];
        // if ( ! pte->present) {
        //     // this in reality generates the page fault exception and now you execute // verify this is actually a valid page in a vma if not raise error and next inst
        //     frame_t *newframe = get_frame();
        //     //-> figure out if/what to do with old frame if it was mapped
        //     // see general outline in MM-slides under Lab3 header and writeup below
        //     // see whether and how to bring in the content of the access page.
        // } // check write protection
        // // simulate instruction execution by hardware by updating the R/M PTE bits
        // update_pte(read/modify) bits based on operations.
    }
}

int main(int argc, char** argv) {
    vector <VMA*> vtable;
    VMA *vma;
    Process* proc;
    int c, x;
    char *num_frames, *algo, *options;
    string rfile, infile, intxt;
    while((c = getopt(argc, argv, "f:a:o:")) != -1) {
        switch(c) {
            case 'f':
                sscanf(optarg, "%d", &frame_count);
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
    
    switch((char)*algo) {
        case 'f':
            cout<<"FIFO\n";
            THE_PAGER = new FIFO(frame_count);
    }
    // generateFreeList();

    rfile = argv[argc-2];
    infile = argv[argc-1];
    
    // Reading from rfile and storing
    myfile.open(rfile);
    getline(myfile, s_temp);
    sscanf(&s_temp[0], "%d", &ran_count);
    while(getline(myfile, s_temp)) {
        sscanf(&s_temp[0], "%d", &x);
        randvals.push_back(x);
    }
    myfile.close();

    // Reading from inputfile and storing
    myfile.open(infile);
    intxt = getLineV2();
    sscanf(&intxt[0], "%d", &proc_count);
    int pi = 0, n_vma;
    int svp, evp, wp, fm;
    for(int i = 0; i < proc_count; i++) {
        intxt = getLineV2();
        sscanf(&intxt[0], "%d", &n_vma);
        vtable.clear();
        for(int j = 0; j < n_vma; j++) {
            intxt = getLineV2();
            sscanf(&intxt[0], "%d %d %d %d", &svp, &evp, &wp, &fm);
            vma = new VMA(svp, evp, wp, fm);
            vtable.push_back(vma);
        }
        proc = new Process(i, vtable);
        proc_list.push_back(proc);
    }
    char c1;
    int i1;
    Simulation();
    //reading instructions
    // intxt = getLineV2();
    // while(intxt[0] != '#' and !myfile.eof()) {
    //     sscanf(&intxt[0], "%c %d", &c1, &i1);
    //     instructions.push_back(make_pair(c1, i1));
    //     intxt = getLineV2();
    // }
    myfile.close();

    // printVMAs();
    // printInstrs();

    return 0;
}