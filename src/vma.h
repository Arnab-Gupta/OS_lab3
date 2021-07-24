#include <vector>

class VMA {
    int start_vpage;
    int end_vpage;
    int write_protected; // check if this should be a boolean ?
    int file_mapped; // check if this should be a boolean ?
};