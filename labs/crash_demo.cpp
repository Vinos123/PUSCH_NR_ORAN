#include <cstdlib>
void inspect_then_abort(int length) {
    volatile int visible_length=length;
    (void)visible_length;
    std::abort(); // Intentional, isolated GDB exercise; no real project defect.
}
int main() { inspect_then_abort(13); }
