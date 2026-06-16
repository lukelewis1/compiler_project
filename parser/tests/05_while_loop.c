class Counter {
    public:
        int count;
        int run(int limit) {
            while (count) {
                count = count + 1;
            }
            return count;
        }
};
