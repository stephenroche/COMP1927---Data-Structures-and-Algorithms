// testList.c - testing DLList data type
// Written by John Shepherd, March 2013

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "DLList.h"

int main(int argc, char *argv[])
{
	DLList myList;
    FILE * f = fopen("text", "r");
	myList = getDLList(f);
	putDLList(stdout,myList);
	assert(validDLList(myList));
	// TODO: more tests needed here
    // Remove first line
    DLListDelete(myList);
    // Add Item to start
    DLListBefore(myList,"Hello");
    assert(validDLList(myList));
    // Move to next line
    DLListMove(myList, 1);
    // Add Item to End
    DLListAfter(myList, "Hello");
    assert(validDLList(myList));
    // MOve to next line
    // Delete item
    // Move to last line
    // Delete last line
    // Delete all the lines in the file
    int i;
    for (i = 0; i < 20; i++) { // txt file must be < 20 lines
        DLListDelete(myList);
        assert(validDLList(myList));
    }

    //ADd item to beginning empty list
    DLListBefore(myList, "Hello");
    assert(validDLList(myList));
    // Delete item from list with 1 item
    DLListDelete(myList);
    assert(validDLList(myList));
    // ADd item to end of empty list
    DLListAfter (myList,  "Bye");
    assert(validDLList(myList));
	putDLList(stdout,myList);
	return 0;
}
