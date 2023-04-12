#include "goodmalloc.hpp"
// Constructor of the GoodMallocMemory class.
GoodMallocMemory::GoodMallocMemory()
{
    mem = nullptr;
    maxFrameCount = 0;
    PT = map<string, PTEntry>();
    scope = "";
    freeFrameHead = -1;
    freeFrameCount = 0;
    varStack = stack<string>();
}
// Create contiguous memory segment to contain all linked lists.
void GoodMallocMemory::createMem(size_t memsize)
{
    if(!mem) throw runtime_error("createMem: Only one memory block to be created per instance of GoodMallocMemory");
    // allocate space in multiple of Element size
    // Assumption: Element size is 2^4 bytes
    maxFrameCount = (memsize>>4)+1;
    mem = (Element*) calloc(maxFrameCount, FRAMESIZE);
    if(!mem) throw runtime_error("createMem: Memory allocation failure");

    // initialize memory with implicit free list
    freeFrameHead = 0;
    freeFrameCount = maxFrameCount;
    for(int i=0; i<maxFrameCount; i++)
    {
        mem[i].prev = i-1;
        if(i==maxFrameCount-1) mem[i].next = -1;
        else mem[i].next = i+1;
    }
    return;
}
// Create a doubly linked list inside contiguous memory.
void GoodMallocMemory::createList(string listname, size_t listlen)
{
    // Error reporting for illegal states
    if(listname.find(" ")!=string::npos || listname.find("$")!=string::npos || listname.find("|")!=string::npos)
        throw runtime_error("createList: Variable name must not contain ' ' or '$' or '|'");
    if(!mem)
        throw runtime_error("createList: Memory not allocated, call createMem() first");
    if(freeFrameCount < listlen)
        throw runtime_error("createList: Not enough free memory for the list");
    if(listlen == 0)
        throw runtime_error("createList: List size must be a positive integer");
    
    string absolute_name = scope + " | " + listname;
    // traverse free list, allocate memory using First Fit
    PTEntry newlist;
    newlist.head = newlist.tail = freeFrameHead;
    newlist.scope = 1;
    for(int i=0; i<(listlen-1); i++)
    {
        if(mem[newlist.tail].next != -1) newlist.tail = mem[newlist.tail].next;
        else throw runtime_error("createList: Went out of free list bounds");
    }
    freeFrameHead = mem[newlist.tail].next;
    if(freeFrameHead != -1) mem[freeFrameHead].prev = -1;
    mem[newlist.tail].next = -1;
    freeFrameCount -= listlen;
    // add list to page table
    PT.insert(make_pair(absolute_name, newlist));
    // push to varStack
    varStack.push(absolute_name);
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
    cout<<"Entering scope "<<func<<endl;
    // prepend with current function name
    scope = func + " " + scope;
    // mark the stack with sentinel string $
    varStack.push("$");
    return;
}
// To be called immediately before exiting a scope.
void GoodMallocMemory::exitScope()
{
    // extract current scope prefix
    string prefix = "";
    size_t pos = scope.find(" "); // Assumption: scope doesn't begin with " "
    if(pos != string::npos)
    {
        prefix = scope.substr(0, pos);
        cout<<"Exiting scope "<<prefix<<endl;
        scope.erase(0, pos+1);
    }
    else scope = "";
    // pop from stack until $ is reached
    while(!varStack.empty())
    {
        string curr = varStack.top();
        varStack.pop();
        if(curr == "$") break; // exit when no more variables to pop from current scope
        if(curr.find(prefix) != 0) // report error if curr is not within scope
            throw runtime_error("exitScope: Non-scope variable in stack");
        // mark the PTEntry of curr as out-of-scope
        if(PT.find(curr) != PT.end())
            PT[curr].scope = 0;
        else throw runtime_error("exitScope: Stack variable not present in page table");
    }
    return;
}