#include <vector>

using namespace std;

struct frame_t {
    unsigned int proc_id: 4;    // since max 10 procs
    unsigned int vpage: 3;      // since max 8 vpages per proc
    unsigned int free: 1;       // indicates whether a particular frame is free or not
    frame_t() : proc_id(0), vpage(0), free(0) {}
};

class Pager {
    public:
        int frame_count;
        int frame_ind;
        vector <frame_t> frame_table;
        virtual frame_t* select_victim_frame() = 0; // virtual base class
};

class FIFO: public Pager {
    public:
        FIFO (int fc) {
            frame_count = fc;
            frame_ind = 0;
            for(int i=0; i < fc; i++) {
                frame_t f;
                frame_table.push_back(f);
            }
        }
        frame_t* select_victim_frame() {
            frame_t *fp;
            fp = &frame_table[frame_ind++];
            return fp;
        }
};