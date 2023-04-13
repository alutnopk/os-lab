#include "goodmalloc.hpp"
#include <iostream>
#include <random>
using namespace std;

GoodMallocMemory M;

int mergeSort(int head)
{
    M.enterScope(__func__);
    // base case
    if(M.frameToPtr(head)->next == -1)
    {
        M.exitScope();
        return head;
    }
    // find middle element
    int slow = head, fast = head;
    while(M.frameToPtr(fast)->next != -1 && M.frameToPtr(M.frameToPtr(fast)->next)->next != -1)
    {
        slow = M.frameToPtr(slow)->next;
        fast = M.frameToPtr(M.frameToPtr(fast)->next)->next;
    }
    int mid = M.frameToPtr(slow)->next;
    // M.frameToPtr(slow)->next = -1;

    // sort left and right halves
    // int left = mergeSort(head);
    // int right = mergeSort(mid);

    // merge sorted halves
    // int sortedHead = -1, sortedTail = -1;
    // while(left != -1 && right != -1)
    // {
    //     if(M.frameToPtr(left)->data < M.frameToPtr(right)->data)
    //     {
    //         if(sortedHead == -1) sortedHead = left;
    //         else M.frameToPtr(sortedTail)->next = left;
    //         sortedTail = left;
    //         left = M.frameToPtr(left)->next;
    //     }
    //     else
    //     {
    //         if(sortedHead == -1) sortedHead = right;
    //         else M.frameToPtr(sortedTail)->next = right;
    //         sortedTail = right;
    //         right = M.frameToPtr(right)->next;
    //     }
    // }
    // if(left != -1)
    // {
    //     if(sortedHead == -1) sortedHead = left;
    //     else M.frameToPtr(sortedTail)->next = left;
    // }
    // if(right != -1)
    // {
    //     if(sortedHead == -1) sortedHead = right;
    //     else M.frameToPtr(sortedTail)->next = right;
    // }


    cout<<M.frameToPtr(head)->data<<" "<<M.frameToPtr(mid)->data<<endl;

    M.exitScope();
    return 0;
}

int main(int argc, char **argv)
{
    M.enterScope(__func__);
    M.createMem(250 * 1024 * 1024);
    M.createList("mylist", 10);
    // assign random elements to list
    random_device rd; mt19937 gen(rd());
    uniform_int_distribution<> dist(1, 100000);
    for(int i=0; i<10; i++) // optional TODO: improve efficiency
    {
        M.assignVal("mylist", i, dist(gen));
    }
    
    // perform merge sort
    int sortedHead = mergeSort(M.getFrameNo("mylist", 0));
    // print sorted list
    M.printList("mylist");
    M.exitScope();
    
    return 0;
}