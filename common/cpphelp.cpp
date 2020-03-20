#include <stdlib.h>

// c++ memory ops 
void* operator new(size_t sz) { return malloc(sz); }
void operator delete(void* ptr) { free(ptr); }

void* operator new[](size_t sz) { return operator new(sz); }
void operator delete[](void* ptr) { operator delete(ptr); }

void operator delete(void* ptr, size_t) { operator delete(ptr); }
void operator delete[](void* ptr, size_t) { operator delete[](ptr); }
