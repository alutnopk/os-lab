#include "goodmalloc.hpp"

// Create contiguous memory segment to contain all linked lists.
void GoodMallocMemory::createMem(size_t memsize)
{
    // do some malloc'ing into memory
    return;
}
// Create doubly linked list inside contiguous memory.
void GoodMallocMemory::createList(string listname, int listsize)
{
    // if mem not allocated then foff
    return;
}
// Update element of an existing doubly linked list.
void GoodMallocMemory::assignVal(string listname, int offset, int value)
{
    return;
}
// Free the variables which are out-of-scope.
void GoodMallocMemory::freeElem()
{
    return;
}
// Free a particular variable.
void GoodMallocMemory::freeElem(string listname)
{
    return;
}
// Convert frame number to element pointer.
Element* GoodMallocMemory::frameToPtr(int frameno)
{
    return (this->mem + frameno*FRAMESIZE);
}
// Get the frame number of the node at an offset.
int GoodMallocMemory::getFrameNo(string listname, int offset)
{
    return;
}
// Assign to a node using the frame number +/- an offset.
int GoodMallocMemory::setVal(int frameno, int offset, int value)
{
    return;
}
// To be called immediately after entering a scope.
void GoodMallocMemory::enterScope(string func)
{
    return;
}
// To be called immediately before exiting a scope.
void GoodMallocMemory::exitScope()
{
    return;
}