#include <vector>

using namespace std;

struct frame_t {
    unsigned int frame_id: 7;
    unsigned int proc_id: 4;    // since max 10 procs
    unsigned int vpage: 6;      // since max 64 vpages per proc
    unsigned int free: 1;       // indicates whether a particular frame is free or not
    frame_t() : proc_id(0), vpage(0), free(1) {}
};

class Pager {
    public:
        int daemon_count;
        unsigned long long total_cost;
        unsigned long instruction_count;
        unsigned long context_switches_count;
        unsigned long process_exit_count;
        int frame_count;
        int frame_ind;
        vector <frame_t> frame_table;
        vector <int> randvals;
        vector <Process*> proc_list;
        virtual frame_t* select_victim_frame() = 0; // virtual base class
};

class FIFO: public Pager {
    public:
        FIFO (int fc) {
            daemon_count = 0;
            total_cost = 0;
            instruction_count = 0;
            context_switches_count = 0;
            process_exit_count = 0;
            frame_count = fc;
            frame_ind = 0;
            for(int i=0; i < fc; i++) {
                frame_t f;
                f.frame_id = i;
                frame_table.push_back(f);
            }
            proc_list.clear();
            randvals.clear();
        }
        frame_t* select_victim_frame() {
            frame_t *fp;
            fp = &frame_table[frame_ind];
            frame_ind = (frame_ind + 1) % frame_count;
            return fp;
        }
};

class CLOCK: public Pager {
    public:
        CLOCK (int fc) {
            daemon_count = 0;
            total_cost = 0;
            instruction_count = 0;
            context_switches_count = 0;
            process_exit_count = 0;
            frame_count = fc;
            frame_ind = 0;
            for(int i=0; i < fc; i++) {
                frame_t f;
                f.frame_id = i;
                frame_table.push_back(f);
            }
            proc_list.clear();
            randvals.clear();
        }

        frame_t* select_victim_frame() {
            // cout<<"yes\n";
            frame_t *fp;
            fp = &frame_table[frame_ind%frame_count];
            Process* p = proc_list[fp->proc_id];
            while(1) {
                // cout<<"checking... "<<fp->vpage<<endl;
                if (p->page_table[fp->vpage].referenced) {
                    p->page_table[fp->vpage].referenced = 0;
                    // frame_ind = (frame_ind + 1) % frame_count;
                    frame_ind++;
                }
                else {
                    // cout<<"picked "<<fp->vpage<<endl;
                    frame_ind++;
                    return fp;
                }
                fp = &frame_table[frame_ind%frame_count];
                p = proc_list[fp->proc_id];
            }
            return fp;
        }
};

class RANDOM: public Pager {
    public:
        int ofs;
        int random_count;
        RANDOM (int fc, int rc) {
            ofs = 0;
            random_count = rc;
            daemon_count = 0;
            total_cost = 0;
            instruction_count = 0;
            context_switches_count = 0;
            process_exit_count = 0;
            frame_count = fc;
            frame_ind = 0;
            for(int i=0; i < fc; i++) {
                frame_t f;
                f.frame_id = i;
                frame_table.push_back(f);
            }
            proc_list.clear();
            randvals.clear();
        }

        int myRandom() { 
            int r = randvals[ofs % random_count] % frame_count; 
            ofs++;
            return r;
        }        

        frame_t* select_victim_frame() {
            frame_t *fp;
            fp = &frame_table[myRandom()];
            return fp;
        }
};

class ESC_NRU: public Pager {
    public:
        ESC_NRU (int fc) {
            daemon_count = 0;
            total_cost = 0;
            instruction_count = 0;
            context_switches_count = 0;
            process_exit_count = 0;
            frame_count = fc;
            frame_ind = 0;
            for(int i=0; i < fc; i++) {
                frame_t f;
                f.frame_id = i;
                frame_table.push_back(f);
            }
            proc_list.clear();
            randvals.clear();
        }

        void resetReferencedBit() {
            Process* p;
            for(int i=0; i<proc_list.size(); i++) {
                p = proc_list[i];
                for(int j=0; j < N_PTE_ENTRIES; j++) {
                    if (p->page_table[j].present) {
                        p->page_table[j].referenced = 0;
                    }
                }
            }
        }

        frame_t* select_victim_frame() {
            int _class = 0, class_ind, f_ind;
            frame_t *fp;
            fp = &frame_table[frame_ind%frame_count];
            Process* p = proc_list[fp->proc_id];
            f_ind = frame_ind%frame_count;
            while(1) {
                class_ind = 2 * p->page_table[fp->vpage].referenced + p->page_table[fp->vpage].modified;
                if(class_ind <= _class) {
                    frame_ind++;
                    if(daemon_count >= 50) {
                        resetReferencedBit();
                        daemon_count = 0;
                    }
                    return fp;
                }
                else {
                    frame_ind++;
                }
                if(f_ind == frame_ind%frame_count) {
                    _class += 1;
                }
                fp = &frame_table[frame_ind%frame_count];
                p = proc_list[fp->proc_id];
            }
            return fp;
        }
};
