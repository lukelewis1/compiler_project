class A { public: int a; int b; int c; int getA() { return a; } int getB() { return b; } int getC() { return c; } };
class B { public: int x; int y; int getX() { return x; } int getY() { return y; } };
class C { private: int val; public: int getVal() { return val; } int setVal(int v) { val = v; } };
class D { protected: int data; public: int getData() { return data; } };
class E { public: int run() { int i = 0; while (i) { i = i + 1; } return i; } };
