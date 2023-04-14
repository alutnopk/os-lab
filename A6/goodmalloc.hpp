#ifndef __GOODMALLOC_H_
#define __GOODMALLOC_H_

#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <map>
using namespace std;

#define FRAMESIZE 16
typedef struct _Element
{
    long data : 64; // 8-byte integer element
    int prev : 32; // 4-byte
    int next : 32; // 4-byte
} Element; // total size = 16 bytes

// Optional TODO: Reduce page entry size
typedef struct _PTEntry
{
    int head; // 4-byte
    int tail; // 4-byte
    int scope; // 4-byte
} PTEntry; // total size = 12 bytes

class GoodMallocMemory
{
    Element* mem; // allocated memory, interpreted as an array of element-sized frames
    int maxFrameCount; // maximum number of frames in the memory
    map<string, PTEntry> PT; // page table
    // listname -> headidx, tailidx, scope
    string scopeStr; // prefix used to scope list names
    int freeFrameHead; // logical pointer to beginning of implicit free list
    int freeFrameTail; // logical pointer to end of free frame list
    size_t freeFrameCount; // number of free frames
    stack<string> varStack; // global variable stack

    public:
    GoodMallocMemory();
    void createMem(size_t memsize);
    void createList(string listname, size_t listlen);
    void assignVal(string listname, size_t offset, long value);
    void assignValMultiple(string listname, size_t offset, long* values, size_t num);
    void freeElem();
    void freeElem(string listname);
    void printList(string listname);
    Element* frameToPtr(int frameno);
    int getFrameNo(string listname, size_t offset);
    int setVal(int frameno, int offset, long value);
    void enterScope(string func);
    void exitScope();
};

#endif