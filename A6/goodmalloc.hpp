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
    long int data : 64; // 8-byte integer element
    int prev : 32; // 4-byte
    int next : 32; // 4-byte
} Element; // total size = 16 bytes

typedef struct _PTEntry
{
    int head; // 4-byte
    int tail; // 4-byte
    int scope; // 4-byte
} PTEntry; // total size = 12 bytes

class GoodMallocMemory
{
    Element* mem; // allocated memory, interpreted as an array of element-sized frames
    map<string, PTEntry> PT; // page table
    string scope; // suffix used to scope list names
    vector<int> freeFrames; // list of available frame indices
    stack<string> varStack; // global variable stack

    public:
    void createMem(size_t memsize);
    void createList(string listname, int listsize);
    void assignVal(string listname, int offset, int value);
    void freeElem();
    void freeElem(string listname);
    // TODO: add helper functions to append scope to variable names, parse them etc.
    Element* frameToPtr(int frameno);
    int getFrameNo(string listname, int offset);
    int setVal(int frameno, int offset, int value);
    void enterScope(string func);
    void exitScope();
};

#endif