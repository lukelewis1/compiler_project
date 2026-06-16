class Looper {
    public:
        int i;
        int run() {
            for (; i; i) {
                i = i + 1;
            }
            return i;
        }
};
