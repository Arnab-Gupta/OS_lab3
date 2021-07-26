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
        unsigned long long total_cost;
        unsigned long instruction_count;
        unsigned long context_switches_count;
        unsigned long process_exit_count;
        int frame_count;
        int frame_ind;
        vector <frame_t> frame_table;
        virtual frame_t* select_victim_frame() = 0; // virtual base class
};

class FIFO: public Pager {
    public:
        FIFO (int fc) {
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
        }
        frame_t* select_victim_frame() {
            frame_t *fp;
            fp = &frame_table[frame_ind%frame_count];
            frame_ind++;
            return fp;
        }
};