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

    pte_t (): present(0), referenced(0), modified(0), write_protect(0), pagedout(0), frame_no(0) {} 
};


class Process {
    
    public:
        int pid;
        vector <VMA*> vma_table;
        vector <pte_t> page_table;
    
        Process (int id, vector <VMA*> vtable) {
            pid = id;
            vma_table = vtable;
            struct pte_t pte;
            for(int i = 0; i < N_PTE_ENTRIES; i++) {
                page_table.push_back(pte);
            }
        };   
};
