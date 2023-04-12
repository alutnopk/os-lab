#include "goodmalloc.hpp"
#include <iostream>
#include <random>
using namespace std;

GoodMallocMemory M;

int mergeSort(int head)
{
    M.enterScope(__func__);
    
    M.exitScope();
}

int main(int argc, char **argv)
{
    M.enterScope(__func__);
    M.createMem(250 * 1024 * 1024);
    M.createList("mylist", 50000);
    // assign random elements to list
    random_device rd; mt19937 gen(rd());
    uniform_int_distribution<> dist(1, 100000);
    for(int i=0; i<50000; i++) // optional TODO: improve efficiency
    {
        M.assignVal("mylist", i, dist(gen));
    }
    // perform merge sort
    int sortedHead = mergeSort(M.getFrameNo("mylist", 0));
    M.exitScope();
    return 0;
}