class Manager {
    public:
        int create() {
            int obj = new Counter();
            delete obj;
            return 0;
        }
};
