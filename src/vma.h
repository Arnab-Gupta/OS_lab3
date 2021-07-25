class VMA {
    public:
        int start_vpage;
        int end_vpage;
        int write_protected; // check if this should be a boolean ?
        int file_mapped; // check if this should be a boolean ?

        VMA (int svp, int evp, int wp, int fm) {
            start_vpage = svp;
            end_vpage = evp;
            write_protected = wp;
            file_mapped = fm;
        }
};