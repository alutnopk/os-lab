#include "goodmalloc.hpp"
#include <iostream>
#include <random>
using namespace std;

GoodMallocMemory M;

int merge(int left, int right)
{
    M.enterScope(__func__);
    if(left == -1)
    {
        M.exitScope();
        return right;
    }
    if(right == -1)
    {
        M.exitScope();
        return left;
    }

    int mergedlist = -1;
    if(M.frameToPtr(left)->data < M.frameToPtr(right)->data)
    {
        // mergedlist = left;
        // M.frameToPtr(mergedlist)->next = merge(M.frameToPtr(left)->next, right);
        // M.frameToPtr(M.frameToPtr(mergedlist)->next)->prev = mergedlist;
        // M.exitScope();
        // return mergedlist;

        M.frameToPtr(left)->next = merge(M.frameToPtr(left)->next, right);
        M.frameToPtr(M.frameToPtr(left)->next)->prev = left;
        M.frameToPtr(left)->prev = -1;
        M.exitScope();
        return left;
    }
    else
    {
        // mergedList = right;
        // M.frameToPtr(mergedlist)->next = merge(left, M.frameToPtr(right)->next);
        // M.frameToPtr(M.frameToPtr(mergedlist)->next)->prev = mergedlist;
        // M.exitScope();
        // return mergedlist;

        M.frameToPtr(right)->next = merge(left, M.frameToPtr(right)->next);
        M.frameToPtr(M.frameToPtr(right)->next)->prev = right;
        M.frameToPtr(right)->prev = -1;
        M.exitScope();
        return right;
    }
}

int mergeSort(int head)
{
    M.enterScope(__func__);
    // base case
    if(head == -1 || M.frameToPtr(head)->next == -1)
    {
        M.exitScope();
        return head;
    }
    // find middle element
    int slow = head, fast = head;
    while (M.frameToPtr(fast)->next != -1 && M.frameToPtr(M.frameToPtr(fast)->next)->next != -1)
    {
        slow = M.frameToPtr(slow)->next;
        fast = M.frameToPtr(M.frameToPtr(fast)->next)->next;
    }
    // split the list in-place
    int mid = M.frameToPtr(slow)->next;
    M.frameToPtr(slow)->next = -1;

    // sort left and right sublists
    head = mergeSort(head);
    mid = mergeSort(mid);

    int result = merge(head, mid);
    M.exitScope();
    return result;
}


int main(int argc, char **argv)
{
    // M = new GoodMallocMemory();
    try
    {
        M.enterScope(__func__);
        M.createMem(250 * 1024 * 1024);
        M.createList("mylist", 50000);
        // assign random elements to list
        random_device rd; mt19937 gen(rd());
        uniform_int_distribution<> dist(1, 100000);
        for(int i=0; i<50000; i++)
        {
            M.assignVal("mylist", i, dist(gen));
        }
        M.printList("mylist");

        // perform merge sort
        int sortedHead = mergeSort(M.getFrameNo("mylist", 0));
        M.reassign("mylist", sortedHead);
        cout<<"--------------------------------------------------------------------"<<endl;

        M.printList("mylist");
        M.exitScope();
        // delete M;
        return 0;
    }
    catch(exception &e){cerr<<e.what()<<endl; exit(EXIT_FAILURE);}
}