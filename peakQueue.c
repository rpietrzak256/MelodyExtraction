#include <stdlib.h>
#include <stdio.h>
#include "peakQueue.h"

// Implementing a minimum priority queue based on a peak's amplitude (value of
// peakY)

struct peakQueue peakQueueNew(int max_size)
{
	// constructor for peakQueue
	struct peakQueue peakQ;
	peakQ.array = malloc(max_size*sizeof(struct peak));
	peakQ.max_size = max_size;
	peakQ.cur_size = 0;
	return peakQ;
}

void peakQueueDestroy(struct peakQueue peakQ)
{
	// destructor for peakQueue
	free(peakQ.array);
}


void peakQueueSwapPeaks(struct peakQueue *peakQ, int index1, int index2)
{
	// helper function that swaps the peak at index1 with the peak at index
	// 2
	struct peak tempPeak;
	tempPeak = peakQ->array[index1];
	peakQ->array[index1] = peakQ->array[index2];
	peakQ->array[index2]=tempPeak;
}

void peakQueueBubbleUp(struct peakQueue *peakQ,int index)
{	
	// this helper function is called after a new peak has been inserted
	// this restores the heap properties
	// compare the peak at index with its parent
	// the parent has (index-1)/2
	int parentIndex = (index-1)/2;
	
	//check to see if the 
	if (peakQ->array[parentIndex].peakY > peakQ->array[index].peakY){
		peakQueueSwapPeaks(peakQ, index, parentIndex);
		if (parentIndex>0){
			peakQueueBubbleUp(peakQ,parentIndex);
		}
	}
}

void peakQueueBubbleDown(struct peakQueue *peakQ, int index)
{
	// this helper function is called after the minimum has been 
	// replaced with the peak at the last level. This restores 
	// the heap properties

	// Basically we compare the peak at index with both of its 
	// children nodes. If there is a child small than the current 
	// peak, we swap current peak with the child. If both children 
	// are smaller than the current peak, we swap the current 
	// peak with the smaller child.

	// calculate the indices of the children

	int leftIndex = 2*index+1;
	int rightIndex = 2*index+2;

	if (leftIndex >= peakQ->cur_size){
		// in this scenario, the current node has no children
		return;
	} else if (rightIndex>= peakQ->cur_size){
		// in this scenario, the current node only has 1 child
		if (peakQ->array[index].peakY > peakQ->array[leftIndex].peakY){
			peakQueueSwapPeaks(peakQ, index, leftIndex);
		}
		return;
	} else if (peakQ->array[index].peakY > peakQ->array[leftIndex].peakY) {
		// in this scenario, the current node has a smaller peak than 
		// the left child
		if (peakQ->array[leftIndex].peakY>peakQ->array[rightIndex].peakY) {
			// this means the right child is even smaller than the
			// left child
			peakQueueSwapPeaks(peakQ, index, rightIndex);
			peakQueueBubbleDown(peakQ, rightIndex);
		} else {
			// the parent is larger than the left child but not the
			// right child
			peakQueueSwapPeaks(peakQ, index, leftIndex);
			peakQueueBubbleDown(peakQ, leftIndex);
		}
		return;
	} else if (peakQ->array[index].peakY > peakQ->array[rightIndex].peakY) {
		// the parent is larger than the right child but not the left
		// child
		peakQueueSwapPeaks(peakQ, index, rightIndex);
		peakQueueBubbleDown(peakQ, rightIndex);
	} else {
		// the parent is smaller than both the left and the right child
		return;
	}	
}

void peakQueueInsert(struct peakQueue *peakQ, struct peak newPeak)
{
	// insert a new peak into the peak queue
	peakQ->array[peakQ->cur_size]= newPeak;
	if (peakQ->cur_size != 0){
		peakQueueBubbleUp(peakQ,peakQ->cur_size-1);
	}
	peakQ->cur_size+=1;
}

struct peak peakQueuePop(struct peakQueue *peakQ)
{
	// pop the minimum peak off of the peakQ
	// first check to see if there is only one value
	if (peakQ->cur_size == 1){
		peakQ->cur_size = 0;
		return peakQ->array[0];
	} else {
		// swap the minimum value with the last value
		peakQueueSwapPeaks(peakQ, 0, peakQ->cur_size-1);
		peakQ->cur_size-=1;
		peakQueueBubbleDown(peakQ, 0);
		return peakQ->array[peakQ->cur_size];
	}
}

void peakQueueAddNewPeak(struct peakQueue *peakQ, double peakX, 
			 double peakY)
{
	// mutator method to the peakQ
	// This function is in charge of adding new peak data to the peakQ
	// Basically it keeps adding data until the peakQ is full and it 
	// ensures that the peakQ always contains the largest peaks
	
	if (peakQ->cur_size != peakQ->max_size) {
		// this means that we simply insert another peak into the peakQ
		struct peak newPeak;
		newPeak.peakX = peakX;
		newPeak.peakY = peakY;
		peakQueueInsert(peakQ, newPeak);
	} else {
		// the peakQ is full
		// check the value of peakY of the minimum peak. If it's
		// smaller than the value of peakY for the new peak, we will
		// effectively insert the new peak and pop the minimum peak.
		
		if (peakY > peakQ->array[0].peakY) {
			// We cut out the intermediary steps of inserting then
			// popping, we will just by just updating the
			// information of the minimum peak to reflect the data
			// of the new peak and then we will call
			// peakQueueBubbleDown.
			peakQ->array[0].peakX = peakX;
			peakQ->array[0].peakY = peakY;
			peakQueueBubbleDown(peakQ, 0);
		}
	}
}

int peakQueueToArrays(struct peakQueue *peakQ, double* peakX, double* peakY)
{
	// this removes the data from array struct format and places it in
	// arrays this function returns the number of entries in the arrays
	int i;
	int length = peakQ->cur_size;
	struct peak tempPeak;
	for (i=0;i<length;i++){
		tempPeak = peakQueuePop(peakQ);
		peakX[i] = tempPeak.peakX;
		peakY[i] = tempPeak.peakY;
	}
	return length;
}


void peakQueuePrint(struct peakQueue *peakQ)
{
	// print out the peaks in the Queue
	int i,length;
	
	length = peakQ->cur_size;
	struct peak *temp = malloc(length * sizeof(struct peak));
	printf("[");
	
	if (peakQ->cur_size !=0) {
		temp[0] = peakQueuePop(peakQ);
		printf("frequency = %.0f magnitude = %.0f", temp[0].peakX,
		       temp[0].peakY);
	}
	for (i=1;i<length;i++){
		temp[i] = peakQueuePop(peakQ);
		printf(",\n frequency = %.0f magnitude = %.0f", temp[i].peakX,
		       temp[i].peakY);
	}

	// now put everything back into the Queue
	for (i=0;i<length;i++){
		peakQueueInsert(peakQ, temp[i]);
	}

	printf("]\n");
	free(temp);

}

/*
int main(int argc, char*argv[]){
	// only including this for testing purposes

	struct peakQueue peakQ;
	peakQ = peakQueueNew(5);
	peakQueuePrint(&peakQ);
	printf("\n");
	peakQueueAddNewPeak(&peakQ, 53, 534);
	peakQueuePrint(&peakQ);
	printf("\n");
	peakQueueAddNewPeak(&peakQ, 192, 593);
	peakQueuePrint(&peakQ);
	printf("\n");
	peakQueueAddNewPeak(&peakQ, 190, 603);
	peakQueuePrint(&peakQ);
	printf("\n");

	peakQueueAddNewPeak(&peakQ, 54, 502);
	peakQueuePrint(&peakQ);
	printf("\n");

	peakQueueAddNewPeak(&peakQ, 391, 597);
	peakQueuePrint(&peakQ);
	printf("\n");

	peakQueueAddNewPeak(&peakQ, 265, 586);
	peakQueuePrint(&peakQ);
	printf("\n");

	peakQueueAddNewPeak(&peakQ, 600, 400);
	peakQueuePrint(&peakQ);
	printf("\n");
	
	peakQueueDestroy(peakQ);

	return 0;
}

*/
