#include "goodmalloc.hpp"
#include <iostream>
#include <random>
using namespace std;

#define CREATEMEM_SIZE (250 * 1024 * 1024)
#define CREATELIST_SIZE 50000
#define ELEMENT_MAX 100000

GoodMallocMemory M1;

int mergeSort(int head)
{
    M1.enterScope(__func__);
    
    M1.exitScope();
}

int main(int argc, char **argv)
{
    M1.enterScope(__func__);
    M1.createMem(CREATEMEM_SIZE);
    M1.createList("mylist", CREATELIST_SIZE);
    // assign random elements to list
    random_device rd; mt19937 gen(rd());
    uniform_int_distribution<> dist(1, ELEMENT_MAX);
    for(int i=0; i<CREATELIST_SIZE; i++) // optional TODO: improve efficiency
    {
        M1.assignVal("mylist", i, dist(gen));
    }
    // perform merge sort
    int sortedHead = mergeSort(M1.getFrameNo("mylist", 0));
    
    return 0;
}