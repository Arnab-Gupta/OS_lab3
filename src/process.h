#include <vector>
#include "vma.h"

using namespace std;

#define N_PTE_ENTRIES 64

struct pte_t {
    unsigned int present: 1;
    unsigned int referenced: 1;
    unsigned int modified: 1;
    unsigned int write_protect: 1;
    unsigned int pagedout: 1;
    unsigned int frame_no: 7;   // since we have a max of 128 frames
    // 20 bits left to be used by programmer
    unsigned int vma_no: 3;

    pte_t (): present(0), referenced(0), modified(0), write_protect(0), pagedout(0), frame_no(0) {} 
};


class Process {
    
    public:
        int pid;
        unsigned long unmaps;
        unsigned long maps;
        unsigned long ins;
        unsigned long outs;
        unsigned long fins;
        unsigned long fouts;
        unsigned long zeros;
        unsigned long segv;
        unsigned long segprot;
        vector <VMA*> vma_table;
        vector <pte_t> page_table;
    
        Process (int id, vector <VMA*> vtable) {
            int vno = 0;
            maps = 0;
            ins = 0;
            outs = 0;
            unmaps = 0;
            fins = 0;
            fouts = 0;
            zeros = 0;
            segv = 0;
            segprot = 0;
            pid = id;
            vma_table = vtable;
            struct pte_t pte;
            for(int i = 0; i < N_PTE_ENTRIES; i++) {
                if(i > vtable[vno]->end_vpage) vno++;
                pte.vma_no = vno;
                page_table.push_back(pte);
            }
        };   
};
