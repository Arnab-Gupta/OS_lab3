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

#define READ_WRITE_INST 1
#define CONTEXT_SWITCH 130
#define PROCESS_EXIT 1250
#define MAPS 300
#define UNMAPS 400
#define INS 3100
#define OUTS 2700
#define FINS 2800
#define FOUTS 2400
#define ZEROS 140
#define SEGV 340
#define SEGPROT 420 

// ------------------------------
// Global variables
// ------------------------------
int ran_count;
int proc_count;
int frame_count;
long long instr_ind = -1;
char operation;
int vpage;
vector <pair<char, int>> instructions;
ifstream myfile;
string s_temp;
Pager *THE_PAGER;
Process* CURRENT_PROCESS = nullptr;
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
    for(vector<Process*>::iterator it = THE_PAGER->proc_list.begin(); it != THE_PAGER->proc_list.end(); ++it) {
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
bool checkValidity(int);
void Simulation();
void Summarize();

void generateFreeList() {
    struct frame_t *fp;
    for(int i=0; i < THE_PAGER->frame_count; i++) {
        fp = &THE_PAGER->frame_table[i];
        free_list.push_back(fp);
    }
}

frame_t* allocate_frame_from_free_list() {
    frame_t *fp = nullptr;
    if(!free_list.empty()) {
        fp = free_list.front();
        free_list.pop_front();
    }
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
        instr_ind++;
        return true;
    }
    return false;
}

bool checkValidity(int vpage) {
    VMA *v;
    for(vector<VMA*>::iterator it = CURRENT_PROCESS->vma_table.begin(); it != CURRENT_PROCESS->vma_table.end(); it++) {
        v = *it;
        if(vpage >= v->start_vpage && vpage <= v->end_vpage)
            return true;
    }
    return false;
}

void exitProcess(Process* p) {
    cout<<"EXIT current process "<<p->pid<<endl;
    for(int i=0; i < N_PTE_ENTRIES; i++) {
        if(p->page_table[i].present) {
            cout<<" UNMAP "<<p->pid<<":"<<i<<endl;
            p->unmaps++;
            THE_PAGER->total_cost += UNMAPS;
            THE_PAGER->frame_table[p->page_table[i].frame_no].free = 1;
            free_list.push_back(&THE_PAGER->frame_table[p->page_table[i].frame_no]);
            if(p->page_table[i].modified && p->vma_table[p->page_table[i].vma_no]->file_mapped) {
                cout<<" FOUT\n";
                p->fouts++;
                THE_PAGER->total_cost += FOUTS;
            }
        }
        p->page_table[i].present = 0;
        p->page_table[i].pagedout = 0;
    }
}

void Simulation() {
    while (get_next_instruction(operation, vpage)) {
        THE_PAGER->instruction_count++;
        THE_PAGER->daemon_count++;
        cout<<instr_ind<<": ==> "<<operation<<" "<<vpage<<endl;
        // handle special case of “c” and “e” instruction
        if (operation == 'c') {
            THE_PAGER->context_switches_count++;
            THE_PAGER->total_cost += CONTEXT_SWITCH;
            CURRENT_PROCESS = THE_PAGER->proc_list[vpage];
            continue;
        }
        if (operation == 'e') {
            THE_PAGER->process_exit_count++;
            THE_PAGER->total_cost += PROCESS_EXIT;
            exitProcess(THE_PAGER->proc_list[vpage]);
            continue;
        }
        else {
            THE_PAGER->total_cost += READ_WRITE_INST;
        }

        // checks if the vpage is valid for the current process
        if (!checkValidity(vpage)) {
            CURRENT_PROCESS->segv++;
            THE_PAGER->total_cost += SEGV;
            cout<<" SEGV\n";
            continue;
        }

        // now the real instructions for read and write    
        pte_t *pte = &CURRENT_PROCESS->page_table[vpage];
        if (!pte->present) {
        //     // this in reality generates the page fault exception and now you execute // verify this is actually a valid page in a vma if not raise error and next inst
            frame_t *newframe = get_frame();
            if (!newframe->free) {
                Process *old_proc;
                old_proc = THE_PAGER->proc_list[newframe->proc_id];
                pte_t *old_pte = &old_proc->page_table[newframe->vpage];
                cout<<" UNMAP "<<old_proc->pid<<":"<<newframe->vpage<<endl;
                old_proc->unmaps++;
                THE_PAGER->total_cost += UNMAPS;
                if (old_pte->modified) {
                    old_pte->modified = 0;
                    if(old_proc->vma_table[old_pte->vma_no]->file_mapped) {
                        cout<<" FOUT\n";
                        old_proc->fouts++;
                        THE_PAGER->total_cost += FOUTS;
                    }
                    else {
                        old_pte->pagedout = 1;
                        old_proc->outs++;
                        THE_PAGER->total_cost += OUTS;
                        cout<<" OUT\n";
                    }
                }
                old_pte->modified = 0;
                old_pte->referenced = 0;
                old_pte->present = 0;
            }
            if(pte->pagedout) {
                if(CURRENT_PROCESS->vma_table[pte->vma_no]->file_mapped) {
                    CURRENT_PROCESS->fins++;
                    THE_PAGER->total_cost += FINS;
                    cout<<" FIN\n";
                }    
                else {
                    CURRENT_PROCESS->ins++;
                    THE_PAGER->total_cost += INS;
                    cout<<" IN\n";
                }
            }   
            else 
            {
                if(CURRENT_PROCESS->vma_table[pte->vma_no]->file_mapped) {
                    CURRENT_PROCESS->fins++;
                    THE_PAGER->total_cost += FINS;
                    cout<<" FIN\n";
                }
                else {
                    CURRENT_PROCESS->zeros++;
                    THE_PAGER->total_cost += ZEROS;
                    cout<<" ZERO\n";
                }
            }
            cout<<" MAP "<<newframe->frame_id<<endl;
            newframe->age = 0;
            newframe->last_used = THE_PAGER->instruction_count;
            CURRENT_PROCESS->maps++;
            THE_PAGER->total_cost += MAPS;
            // updating the pte
            pte->frame_no = newframe->frame_id;
            newframe->vpage = vpage;
            newframe->proc_id = CURRENT_PROCESS->pid;
            newframe->free = 0;
            pte->present = 1;
            // cout<<"referencing: "<<CURRENT_PROCESS->pid<<":"<<vpage<<endl;
            
                
        //     //-> figure out if/what to do with old frame if it was mapped
        //     // see general outline in MM-slides under Lab3 header and writeup below
        //     // see whether and how to bring in the content of the access page.
        }
        pte->referenced = 1;
        // check write protection
        if(operation == 'w')
        {
            if(CURRENT_PROCESS->vma_table[pte->vma_no]->write_protected) {
                cout<<" SEGPROT\n";
                CURRENT_PROCESS->segprot++;
                THE_PAGER->total_cost += SEGPROT;
            }
            else {
                pte->modified = 1;
            }
        }
    }
}

void Summarize() {
    Process* p;
    pte_t *pte;
    string str1;
    int star, hash;
    //vtable per process
    for(vector<Process*>::iterator it = THE_PAGER->proc_list.begin(); it != THE_PAGER->proc_list.end(); it++) {
        p = *it;
        cout<<"PT["<<p->pid<<"]: ";
        for(int j=0; j<N_PTE_ENTRIES; j++) {
            str1 = to_string(j) + ":";
            if(!p->page_table[j].present) {
                if(p->page_table[j].pagedout) {
                    str1 = "#";
                }
                else {
                    str1 = "*";
                }
            }
            else {
                if(p->page_table[j].referenced) {
                    str1 += "R";
                }
                else str1 += "-";
                if(p->page_table[j].modified) {
                    str1 += "M";
                }
                else str1 += "-";
                if(p->page_table[j].pagedout) {
                    str1 += "S";
                }
                else str1 += "-";
            }
            str1 += " ";
            cout<<str1;
        }
        cout<<endl;
    }
    //frame_table
    cout<<"FT: ";
    for(int i = 0; i < THE_PAGER->frame_count; i++) {
        if(THE_PAGER->frame_table[i].free) {
            cout<<"* ";
            continue;
        }
        cout<<THE_PAGER->frame_table[i].proc_id<<":"<<THE_PAGER->frame_table[i].vpage<<" ";
    }
    cout<<endl;

    //per process stats
    for(vector<Process*>::iterator it = THE_PAGER->proc_list.begin(); it != THE_PAGER->proc_list.end(); it++) {
        p = *it;
        printf("PROC[%d]: U=%lu M=%lu I=%lu O=%lu FI=%lu FO=%lu Z=%lu SV=%lu SP=%lu\n", p->pid, p->unmaps, p->maps, p->ins, p->outs, p->fins, p->fouts, p->zeros, p->segv, p->segprot);
    }
    
    //summary output
    printf("TOTALCOST %lu %lu %lu %llu %lu\n", THE_PAGER->instruction_count, THE_PAGER->context_switches_count, THE_PAGER->process_exit_count, THE_PAGER->total_cost, sizeof(pte_t));

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
    
    // cout<<"1\n";
    rfile = argv[argc-1];
    infile = argv[argc-2];
    
    // Reading from rfile and storing
    myfile.open(rfile);
    getline(myfile, s_temp);
    sscanf(&s_temp[0], "%d", &ran_count);
    
    switch((char)*algo) {
        case 'f':
            THE_PAGER = new FIFO(frame_count);
            break;
        case 'c':
            THE_PAGER = new CLOCK(frame_count);
            break;
        case 'e':
            THE_PAGER = new ESC_NRU(frame_count);
            break;
        case 'r':
            THE_PAGER = new RANDOM(frame_count, ran_count);
            break;
        case 'a':
            THE_PAGER = new AGING(frame_count);
            break;
        case 'w':
            THE_PAGER = new WORKING_SET(frame_count);
            break;
        default:
            cout<<"Invalid algo option\n";
            exit(-1);
    }
    generateFreeList();

    
    while(getline(myfile, s_temp)) {
        sscanf(&s_temp[0], "%d", &x);
        THE_PAGER->randvals.push_back(x);
    }
    myfile.close();

    


    // cout<<"2\n";
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
        THE_PAGER->proc_list.push_back(proc);
    }
    // cout<<"3\n";
    char c1;
    int i1;
    Simulation();
    
    // cout<<"4\n";
    Summarize();
    // cout<<"5\n";
    return 0;
}