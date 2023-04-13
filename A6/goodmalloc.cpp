#include "goodmalloc.hpp"
// Constructor of the GoodMallocMemory class.
GoodMallocMemory::GoodMallocMemory()
{
    mem = nullptr;
    maxFrameCount = 0;
    PT = map<string, PTEntry>();
    scope = "";
    freeFrameHead = -1;
    freeFrameTail = -1;
    freeFrameCount = 0;
    varStack = stack<string>();
}
// Create contiguous memory segment to contain all linked lists.
void GoodMallocMemory::createMem(size_t memsize)
{
    if(mem) throw runtime_error("createMem: Only one memory block to be created per instance of GoodMallocMemory");
    // allocate space in multiple of Element size
    // Assumption: Element size is 2^4 bytes
    maxFrameCount = (memsize>>4)+1;
    mem = (Element*) calloc(maxFrameCount, FRAMESIZE);
    if(!mem) throw runtime_error("createMem: Memory allocation failure");

    // initialize memory with implicit free list
    freeFrameHead = 0;
    freeFrameTail = maxFrameCount-1;
    freeFrameCount = maxFrameCount;
    for(int i=0; i<maxFrameCount; i++)
    {
        mem[i].prev = i-1;
        if(i==maxFrameCount-1) mem[i].next = -1;
        else mem[i].next = i+1;
    }
    // return;
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
    for(size_t i=0; i<(listlen-1); i++)
    {
        if(mem[newlist.tail].next != -1) newlist.tail = mem[newlist.tail].next;
        else throw runtime_error("createList: Went out of free list bounds");
    }
    // if all free frames used up
    if(newlist.tail == freeFrameTail)
    {
        freeFrameHead = freeFrameTail = -1;
    }
    else
    {
        freeFrameHead = mem[newlist.tail].next;
        mem[freeFrameHead].prev = -1;
    }
    mem[newlist.tail].next = -1;
    freeFrameCount -= listlen;
    // add list to page table
    PT.insert(make_pair(absolute_name, newlist));
    // push to varStack
    varStack.push(absolute_name);
    return;
}
// Update a specific element of a list. Worst case O(n) complexity.
void GoodMallocMemory::assignVal(string listname, size_t offset, long value)
{
    string absolute_name = scope + " | " + listname;
    if(PT.find(absolute_name) == PT.end()) throw runtime_error("assignVal: List does not exist");
    PTEntry& currlist = PT[absolute_name];
    if(currlist.scope == 0) throw runtime_error("assignVal: List is out-of-scope, cannot assign to it");
    // Traverse list and assign element
    int target = currlist.head;
    for(size_t i=0; i<offset; i++)
    {
        if(mem[target].next != -1) target = mem[target].next;
        else throw runtime_error("assignVal: Offset is out of bounds");
    }
    mem[target].data = value;
    return;
}
// assign values to a list for a given offset and number of elements also check if the size of array is equal to number of elements to be updated
void GoodMallocMemory::assignVal(string listname, size_t offset, size_t num, long *values)
{
    int size = 0;
    while (values[size] != -1)
    {
        // cout<<values[size]<<endl;
        size++;
    }
    cout << "size: " << size << endl;
    cout<<"NUM: "<<num<<endl;
    //find size of array
    //check if num is equal to size of array
    if(size != num)
    {
        throw runtime_error("assignVal: Number of elements to be updated is not equal to size of array");
    }
    string absolute_name = scope + " | " + listname;
    if(PT.find(absolute_name) == PT.end()) throw runtime_error("assignVal: List does not exist");
    PTEntry& currlist = PT[absolute_name];
    if(currlist.scope == 0) throw runtime_error("assignVal: List is out-of-scope, cannot assign to it");
    // Traverse list and assign element
    int target = currlist.head;
    for(size_t i=0; i<offset; i++)
    {
        if(mem[target].next != -1) target = mem[target].next;
        else throw runtime_error("assignVal: Offset is out of bounds");
    }
    for(size_t i=0; i<num; i++)
    {
        if(mem[target].next == -1) throw runtime_error("assignVal: Offset is out of bounds");
        mem[target].data = values[i];
        target = mem[target].next;
        
    }
    return;

}


// Free the variables which are out-of-scope.
void GoodMallocMemory::freeElem()
{
    // go through entire PT for entries where scope == 0, and do free on them each
    for(auto it = PT.begin(); it != PT.end();)
    {
        if(it->second.scope == 0) // remove each list which is out-of-scope
        {
            cout<<"Freeing list "<<it->first<<endl;
            PTEntry& currlist = it->second;
            // append this list to freeframes list
            mem[freeFrameTail].next = currlist.head;
            mem[currlist.head].prev = freeFrameTail;
            freeFrameTail = currlist.tail;
            // O(n) updation code for freeFrameCount
            freeFrameCount = 0;
            for(int f=freeFrameHead; f!=-1; f = mem[f].next)
            {
                freeFrameCount++;
            }
            // remove page table entry
            it = PT.erase(it);
        }
        else it++;
    }
    return;
}
// Free a particular list.
void GoodMallocMemory::freeElem(string listname)
{
    string absolute_name = scope + " | " + listname;
    if(PT.find(absolute_name) == PT.end()) throw runtime_error("freeElem: List does not exist");
    PTEntry& currlist = PT[absolute_name];
    // append this list to freeframes list
    mem[freeFrameTail].next = currlist.head;
    mem[currlist.head].prev = freeFrameTail;
    freeFrameTail = currlist.tail;
    // O(n) updation code for freeFrameCount
    freeFrameCount = 0;
    for(int f=freeFrameHead; f!=-1; f = mem[f].next)
    {
        freeFrameCount++;
    }
    // remove page table entry
    PT.erase(absolute_name);
    return;
}
// Print the contents of a list.
void GoodMallocMemory::printList(string listname)
{
    string absolute_name = scope + " | " + listname;
    if(PT.find(absolute_name) == PT.end()) throw runtime_error("printList: List does not exist");
    PTEntry& currlist = PT[absolute_name];
    if(currlist.scope == 0) throw runtime_error("printList: List is out-of-scope, cannot print it");
    // Traverse list and print element
    int target = currlist.head;
    // int k=0;
    while(target != -1)                     // remove k<50 to print entire list !!!!!!!!!!!!!!!!!!!!!!!!!!1
    {
        cout<<mem[target].data<<" "<<endl;
        target = mem[target].next;
        // k++;
    }
    cout<<endl;
    return;
}

// Convert frame number to element pointer.
Element* GoodMallocMemory::frameToPtr(int frameno)
{
    return &mem[frameno];
}
// Get the frame number of the node at an offset.
int GoodMallocMemory::getFrameNo(string listname, size_t offset)
{
    string absolute_name = scope + " | " + listname;
    if(PT.find(absolute_name) == PT.end()) throw runtime_error("getFrameNo: List does not exist");
    PTEntry& currlist = PT[absolute_name];
    if(currlist.scope == 0) throw runtime_error("getFrameNo: List is out-of-scope, cannot access it");
    // Traverse list and get element
    int target = currlist.head;
    for(size_t i=0; i<offset; i++)
    {
        if(mem[target].next != -1) target = mem[target].next;
        else throw runtime_error("getFrameNo: Offset is out of bounds");
    }
    return target;
}
// Assign to a node using the frame number +/- an offset.
int GoodMallocMemory::setVal(int frameno, int offset, long value)
{
    int target = frameno;
    for(int i=0; i<offset; i++)
    {
        if(mem[target].next != -1) target = mem[target].next;
        else throw runtime_error("setVal: Offset is out of bounds");
    }
    mem[target].data = value;
    return target;
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
        // there may be stack variables which are absent from the page table, presumably removed using freeElem(name)
        // else throw runtime_error("exitScope: Stack variable not present in page table");
    }
    return;
}