#include "detFuncCore.h"


typedef int (*dFCProcessChunkFunc)(detFuncCore *dFC);
/* The following functions are tracked by detFuncCore based on its state. */
int detFuncCoreProcessNoChunk(detFuncCore *dFC);
int detFuncCoreProcessFirstChunk(detFuncCore *dFC);
int detFuncCoreProcessNormalChunk(detFuncCore *dFC);
int detFuncCoreProcessLastChunk(detFuncCore *dFC);
int detFuncCoreProcessSingleChunk(detFuncCore *dFC);

typedef int (*dFCSetInputChunkFunc)(detFuncCore *dFC,  float* input, int length,
				    int final_chunk);

struct detFuncCore{
	enum streamState state;
	dFCProcessChunkFunc processChunkFunc;

	int numChannels;
	int hopsize;
	int corrWinSize;
	float *pooledSummaryMatrix;
	int pSMLength;

	// the following 2 variables are defined for the purposes of
	// identifying where the stream terminates. We could come up with a way
	// to determine this without tracking these quantities.
	long streamLength;
	int pSMWinProcessed;
	int terminationIndex;

	float lastPSMEntry;
	int detFuncSize;
	int detFuncFilledLength;
	float *detFunc;

	int dedicatedThreads;

	tripleBuffer *tB;
	filterBank *fB;
	sigOpt *sO;
};

/* A consequence of using subobjects is that the majority of the information 
 * necessary for computing values are stored in the subobjects.
 * It is not always clear to me whether we should be duplicating this 
 * information in the detFuncCore struct, or accessing it from the subobjects. 
 * (For intensive operations the former might be better, but otherwise the 
 * latter is typically better). 
 *
 * I want to be able to modify these decisions in the future so I have factored 
 * them out into the following static inline functions.
 */

// Could get the following from sigOpt
static inline int detFuncCoreHopsize(detFuncCore *dFC){
	return dFC->hopsize;
}

// Could get this from sigOpt - its equal to the number of sigmas
static inline int detFuncCorePSMLength(detFuncCore *dFC){
	return dFC->pSMLength;
}

// Could get the following from any subObject
static inline int detFuncCoreNumChannels(detFuncCore *dFC){
	return dFC->numChannels;
}


/* Some utility functions that will likely be repeated frequently */

static inline float* detFuncCoreGetPSM(detFuncCore *dFC, int thread_num){
	return ((dFC->pooledSummaryMatrix) +
		(thread_num * detFuncCorePSMLength(detFuncCore *dFC)));
}



/* Onto implementing the interface */

detFuncCore *detFuncCoreCreate(int correntropyWinSize, int hopsize,
			       int numChannels, int sigWinSize,
			       float scaleFactor, int samplerate,
			       float minFreq, float maxFreq,
			       int dedicatedThreads){
	if (dedicatedThreads !=0){
		// temporary
		return NULL;
	}

	//temporary
	//need to assign initial values to streamLength, pSMWinProcessed,
	//    terminationIndex and lastPSMEntry
	return NULL;

	// CHECK THAT ALL OF ARGUMENTS that this function explicitly uses (the
	// subobjects can handle arguments that are simply passed through)


	// Begin the Creation
	detFuncCore *dFC = malloc(sizeof(detFuncCore)):

	if (dFC == NULL){
		return NULL;
	}

	// we should probably think about this more, but for now we set the
	// center of the region for which we compute sigma to be the following
	int initialOffset = correntropyWinSize/2;

	// first contruct sigOpt
	dFC->sO = sigOptCreate(sigWinSize, hopsize, initialOffset,
			       numChannels, scaleFactor);

	if (dFC->sO == NULL){
		free(dFC);
		return NULL;
	}

	// construct the pooledSummaryMatrix
	int pSMLength = sigOptGetSigmasPerBuffer(dFC->sO);
	if (dedicatedThreads == 0){
		dFC->pooledSummaryMatrix = malloc(sizeof(pSMLength));
	} else {
		dFC->pooledSummaryMatrix = malloc(sizeof(pSMLength)
						  * dedicatedThreads);
	}

	// should probably using calloc - otherwise set all values to 0
	free(dFC->pooledSummaryMatrix);

	if (dFC->pooledSummaryMatrix == NULL){
		sigOptDestroy(dFC->sO);
		free(dFC);
		return NULL;
	}

	/* Now calculate the bufferLength and construct the tripleBuffer
	 * The addition of 1 is intentional - correntropy is calculated using
	 * the 2* correntropyWinSize elements AFTER the starting index. */
	int bufferLength = pSMLength * 2 * correntropyWinSize + 1;
	dFC->tB = tripleBufferCreate(numChannels, bufferLength);
	if (dFC->tB == NULL){
		free(dFC->pooledSummaryMatrix);
		sigOptDestroy(dFC->sO);
		free(dFC);
		return NULL;
	}

	int overlap = bufferLength - sigOptGetBufferLength(dFC->sO);
	// Construct the filterBank
	dFC->fB = filterBankNew(numChannels, bufferLength, overlap,
				samplerate, minFreq, maxFreq);

	if (dFC->fB == NULL){
		free(dFC->pooledSummaryMatrix);
		sigOptDestroy(dFC->sO);
		tripleBufferDestroy(dFC->tB);
		free(dFC);
		return NULL;
	}

	dFC->numChannels = numChannels;
	dFC->hopsize = hopsize;
	dFC->corrWinSize = correntropyWinSize;
	dFC->pSMLength = pSMLength;
	dFC->numThreads = numThreads;

	dFC->state = NO_CHUNK;
	dFC->processChunkFunc = &detFuncCorePreprocess;

	// we should actually set a real value for detFunc
	// this needs to be changed
	dFC->detFunc =NULL;
	dFC->detFuncSize = 0;
	dFC->detFuncFilledLength = 0;
	return dFC;
}

void *detFuncCoreDestroy(detFuncCore *dFC){
	if (dFC->detFunc != NULL){
		// we will set detFunc to NULL after returning it
		free(dFC->detFunc);
	}

	free(dFC->pooledSummaryMatrix);
	tripleBufferDestroy(dFC->tB);
	sigOptDestroy(dFC->sO);
	filterBankDestroy(dFC->fB);
	free(dFC);
}

int detFuncCoreFirstChunkLength(detFuncCore *dFC){
	return filterBankFirstChunkLength(dFC->fB);
}

int detFuncCoreNormalChunkLength(detFuncCore *dFC){
	return filterBankNormalChunkLength(dFC->fB);
}


/* I think we are going to add another state aspect to filterBank. It's going 
 * to be related to the value of dedicatedThreads.
 * This involves 2 functions: 
 *    - detFuncCoreSetInputChunk: an interface function
 *    - detFuncCorePullNextChunk: an internal helper function.
 * 
 * If there are no dedicated threads, then all input chunks are added to 
 * detFuncCore via detFuncCoreSetInputChunk. detFuncCorePullNextChunk does 
 * nothing and always succeeds
 *
 * If there is at least 1 dedicated thread, then the detFuncCoreSetInputChunk 
 * should never be called. Instead, the chunk will be provided through some 
 * external streamBuffer. It is accessed by calling detFuncCorePullNextChunk.
 */


/* Below is a helper function that:
 * - control state transitions
 * - advances the tripleBuffer (either adds new buffer or cycles it)
 * - calls sigOptAdvanceBuffer(dFC->sO) (not sure if this gets called 
 *   when adding first 2 chunks)
 * - adds the new input chunk in filterBank
 */
int detFuncCorePrepareNextChunk(detFuncCore *dFC, float *input, int length,
				int final_chunk){
	// update dFC->streamLength appropriately


	
	return -1;
}


int detFuncCoreSetInputChunk(detFuncCore* dFC, float* input, int length,
			     int final_chunk){

	if (dFC->dedicatedThreads == 0){
		return detFuncCorePrepareNextChunk(dFC, input, length,
						   final_chunk);
	} else {
		/* This always automatically fails. This is not the way to pass
		 * the input chunks to detFuncCore if it has dedicated threads. 
		 */
		printf("The function, detFuncCoreSetInputChunk, should not be "
		       "called if dFC has at least 1 dedicated thread.\n");
		return 0;
	}
}

/* This is a helper function. It is used for retrieving the next Chunk if the 
 * detFuncCore has at least 1 dedicated thread. If it doesn't then, this 
 * function just returns True; the same role is carried out by the interface 
 * function detFuncCoreSetInputChunk 
*/
int detFuncCorePullNextChunk(detFuncCore* dFC, float* input, int length,
			     int final_chunk){

	if (dFC->dedicatedThreads == 0){
		return 1;
	} else {
		/* Pulls the next chunk from the external Stream Buffer, if 
		 * applicable.
		 * If the next chunk is not yet ready (unlikely), hang until it
		 * is ready.
		 * Pass the next chunk to detFuncCorePrepareNextChunk
		 *
		 * Needs to be implemented
		 */
		printf("Have not implemented the helper function, "
		       "detFuncCorePullNextChunk, \nfor use when detFuncCore "
		       "has at least 1 dedicated thread.\n"); 
		return -1;
	}
}

/* The bottom is a helper function. For some thread, it computes the 
 * contribution of channel to the PSM for an entire input chunk.
 * If the chunk for which the PSM is being calculated is the final input chunk,
 * then needs to take slightly special care to only compute PSM Contrib to the 
 * end of the chunk (and not any farther)
 *
 * In this function: 
 * first get the trailingBuffer, centralBuffer, and leadingBuffer
 * for every element we wish to fill in PSMcontrib:
 *     sigma= sigOptAdvanceWindow(sO, trailingBuffer,
 *                                centralBuffer, leadingBuffer,
 *                                channel);
 *     set element in PSMcontrib equal to itself + PSM contribution from the 
 *         channel being processed
 * 
 */
int calcPSMContrib(detFuncCore *dFC, int channel, int thread, int numPSMWin){
	return 0;
}


/* The following is a helper function that either sets up sigOpt or computes 
 * correntropy. The choice between these 2 functionalities is determined by the
 * value passed to numPSMWin. If the value is 0, then sigOpt is set up. 
 * Otherwise, calcPSMContrib is called to compute correntropy contributions in 
 * numPSMWin Correntropy calculations.
 * The argument fBfunc controls if and how the tripleBuffer is modified for 
 * every channel. If we are processing the very last input chunk, and the 
 * tripleBuffer has already been modified with the last input, then fBfunc 
 * should be set to NULL. If we are treating a normal chunk or the very first 
 * chunk, then fBfunc should be set to filterBankProcessInput. If the final 
 * input chunk was at least as long as detFuncCoreNormalChunkLength, then we 
 * can also set fBfunc to filterBankPropogateFinalOverlap.
 *
 * If we adopt a simple master-slave thread pool parallelization model, then 
 * the following function must simply be modified.
 */

typedef int (*fBProcessPtr)(filterBank *fB, tripleBuffer *tB, int channel);

int detFuncCoreProcessInputHelper(detFuncCore *dFC, fBProcessPtr fBfunc,
				  int numPSMWin){
	if (dFC->dedicatedThreads != 0){
		printf("Have not implemented the helper function, "
		       "detFuncCoreProcessInputHelper, \nfor use when "
		       "detFuncCore has at least 1 dedicated thread.\n"); 
		return -1;
	}

	for (int channel = 0; channel<detFuncCoreNumChannels(dFC); channel++){
		int temp;
		if (fBfunc != NULL){
			temp = fBfunc(dFC->fB, dFC->tB, channel);
			if (temp!=1){
				printf("There has been a problem with "
				       "execution of fBfunc\n");
				return temp;
			}
		}

		if (numPSMWin>0){
			temp = calcPSMContrib(dFC, channel, 0, numPSMWin);
			if (temp!=1){
				printf("There has been a problem with "
				       "calcPSMContrib\n");
				return temp;
			}
		} else {
			temp = sigOptSetup(dFC->sO, channel,
					   tripleBufferGetBufferPtr(dFC->tB, 0,
								    channel));
			if (temp!=1){
				printf("There has been a problem with "
				       "sigOptSetup\n");
				return temp;
			}
		}
	}
	return 1;
}

int detFuncCoreProcess(detFuncCore* dFC, int thread_num){
	if (dFC->dedicatedThreads == 0){
		dFC->processChunkFunc(dFC);
	} else {
		/* If we decide to use a master slave thread pool, the idea is 
		 * that we would assign one of the threads the master status.
		 */
		printf("Have not implemented the helper function, "
		       "detFuncCorePullNextChunk, \nfor use when detFuncCore "
		       "has at least 1 dedicated thread.\n");
		return -1;
	}
}

/* The following function is a helper function in which all information about 
 * the pooledSummaryMatrix is consolidated. This means that:
 * - In the threaded case, the entries of PSMbuffers are all added together in 
 *   the entries of the PSMbuffer of a single thread.
 */

int detFuncCoreConsolidatePSMEntries(detFuncCore *dFC){
	if (dFC->dedicatedThreads == 0){
		return 1;
	} else {
		printf("Have not implemented the helper function, "
		       "detFuncCoreConsolidatePSMInfo, \nfor use when "
		       "detFuncCore has at least 1 dedicated thread. \n");
		/* Need to add the pooledSummaryMatrices together so that one 
		 * thread has a pooledSummaryMatrix with all accumulated 
		 * entries. This can be done entirely sequentially or partially
		 * parralellized. 
		 */
		return -1;
	}
}

int updateDetFunc(detFuncCore *dFC){
	// Handles the updating of detection function
	// stores the final entry in pooledSummaryMatrix for later
	printf("Have not implemented the helper function, updateDetFunc\n");
	return -1;
}

int flushPSMEntries(detFuncCore *dFC){
	// Handles the process of setting all entries in pooledSummaryMatrix
	// to 0
	if (dFC->dedicatedThreads == 0){
		printf("Have not implemented the helper function, "
		       "flushPSMEntries\n");
		return -1;
	} else {
		printf("Have not implemented the helper function, "
		       "flushPSMEntries, \nfor use when "
		       "detFuncCore has at least 1 dedicated thread. \n");
		return -1;
	}
}

int detFuncCoreProcessPenultimateSectionHelper(detFuncCore *dFC,
					       fBProcessPtr fBfunc){
	/* This is a helper function called by detFuncCoreProcessLastChunk or
	 * detFuncCoreProcessSingleChunk
	 */

	/* Need to implement where we get the terminationIndex from */
	int terminationIndex = dFC->terminationIndex;

	/* Set the terminationIndex in the tripleBuffer and sigOpt */
	int temp = tripleBufferSetTerminalIndex(dFC->tB, terminationIndex);
	if (temp != 1){
		printf("Error while calling tripleBufferSetTerminalIndex\n");
		return temp;
	}

	temp = sigOptSetTerminationIndex(dFC->sO,terminationIndex);
	if (temp != 1){
		printf("Error while calling sigOptSetTerminationIndex\n");
		return temp;
	}

	/* Process the section */
	temp = detFuncCoreProcessInputHelper(dFC, fBfunc, dfC->pSMLength);
	if (temp != 1){
		return temp;
	}

	/* Update detection function */
	temp = updateDetFunc(dFC);
	if (temp != 1){
		return temp;
	}

	/* Flush PSM Entries. */
	temp = flushPSMEntries(dFC);
	if (temp != 1){
		return temp;
	}
	return 1;
}

int detFuncCoreProcessFinalSectionHelper(detFuncCore *dFC){
	/* This is a helper function called by detFuncCoreProcessLastChunk or
	 * detFuncCoreProcessSingleChunk */

	if (tripleBufferNumBuffers(dFC->tB) == 3){
		tripleBufferRemoveTrailingBuffer(tB);
	}

	/* Need to implement where we get the number of finalPSMWindows from */
	int finalPSMWindows = -1;

	/* Process the section */
	temp = detFuncCoreProcessInputHelper(dFC, fBfunc, finalPSMWindows);
	if (temp != 1){
		return temp;
	}

	/* Update detection function */
	temp = updateDetFunc(dFC);
	if (temp != 1){
		return temp;
	}

	return 1;
}

int detFuncCoreProcessNoChunk(detFuncCore *dFC){
	return 0;
}

int detFuncCoreProcessFirstChunk(detFuncCore *dFC){
	return detFuncCoreProcessInputHelper(dFC, filterBankProcessInput, 0);
}

int detFuncCoreProcessNormalChunk(detFuncCore *dFC){
	/* compute the PooledSummary Matrix entries */
	int temp = detFuncCoreProcessInputHelper(dFC, filterBankProcessInput,
						 dfC->pSMLength);
	if (temp != 1){
		return temp;
	}
	/* The following is only important if the detection Function Core is 
	 * parallelized using a master-slave thread pool:
	 * temp = detFuncCoreConsolidatePSMEntries(dFC);
	 * if (temp != 1){
	 *	return temp;
	 * }
	 */

	/* Update detection function */
	temp = updateDetFunc(dFC);
	if (temp != 1){
		return temp;
	}

	/* Flush PSM Entries. */
	temp = flushPSMEntries(dFC);
	if (temp != 1){
		return temp;
	}
	return 1;
}

int detFuncCoreProcessLastChunk(detFuncCore *dFC){

	int temp;
	fBProcessPtr fBfunc;
	if (dFC->terminationIndex >= detFuncCoreNormalChunkLength(dFC)){
		temp = detFuncCoreProcessNormalChunk(dFC);
		if (temp != 1){
			return temp;
		}

		if (tripleBufferNumBuffers(dFC->tB) == 3){
			tripleBufferCycle(dFC->tB);
		} else {
			tripleBufferAddLeadingBuffer(dFC->tB);
		}

		temp = sigOptAdvanceBuffer(dFC->sO);
		if (temp != 1){
			printf("An error occured during the first "
			       "sigOptAdvanceBuffer\n");
			return temp;
		}

		fBfunc = filterBankPropogateFinalOverlap;
		dFC->terminationIndex -= detFuncCoreNormalChunkLength(dFC);
	} else {
		fBfunc = filterBankProcessInput;
	}

	temp = detFuncCoreProcessPenultimateSectionHelper(dFC,fBfunc);
	if (temp != 1){
		return temp;
	}

	temp = sigOptAdvanceBuffer(dFC->sO);
	if (temp != 1){
		printf("An error occured during sigOptAdvanceBuffer\n");
		return temp;
	}

	return detFuncCoreProcessFinalSectionHelper(dFC);
}

int detFuncCoreProcessSingleChunk(detFuncCore *dFC){

	int temp;
	int terminationIndex = dFC->terminationIndex;

	if (terminationIndex >= detFuncCoreNormalChunkLength(dFC)){
		/* The entire stream is longer than a normal ChunkLength (but 
		 * smaller than the expected length of the first chunk). This 
		 * means we need to filter the input, transfer as much as 
		 * possible into the first tripleBuffer buffer, preprocess 
		 * sigOpt and then transfer the remainder into the second 
		 * tripleBuffer. 
		 *
		 * If we won't do this then sigOpt will not account for the 
		 * stream that occurs after the normal ChunkLength in its 
		 * calculations and it's possible that correntropy calculations
		 * involving windows starting after the normal Chunk Length 
		 * will be missed. */
		
		/* need to filter the stream and handle sigOpt preprocessing */
		temp = detFuncCoreProcessInputHelper(dFC,
						     filterBankProcessInput, 0);
		if (temp != 1){
			return temp;
		}

		// Now, update the terminationIndex
		dFC->terminationIndex -= detFuncCoreNormalChunkLength(dFC);
		terminationIndex = dFC->terminationIndex;

		/* Now, let's propogate the remaining stream after normal Chunk
		 * Length and compute correntropy */
		fBfunc = filterBankPropogateFinalOverlap;

		temp = detFuncCoreProcessPenultimateSectionHelper(dFC,fBfunc);
		if (temp != 1){
			return temp;
		}

		temp = sigOptAdvanceBuffer(dFC->sO);
		if (temp != 1){
			printf("An error occured during sigOptAdvanceBuffer\n");
			return temp;
		}
	} else {
		/* Set the terminationIndex in the tripleBuffer and sigOpt */
		int temp = tripleBufferSetTerminalIndex(dFC->tB,
							terminationIndex);
		if (temp != 1){
			printf("Error while calling "
			       "tripleBufferSetTerminalIndex\n");
			return temp;
		}

		temp = sigOptSetTerminationIndex(dFC->sO,terminationIndex);
		if (temp != 1){
			printf("Error while calling "
			       "sigOptSetTerminationIndex\n");
			return temp;
		}

		/* need to filter the stream and handle sigOpt preprocessing */
		temp = detFuncCoreProcessInputHelper(dFC,
						     filterBankProcessInput, 0);
		if (temp != 1){
			return temp;
		}
	}

	/* Now lets process the input in the last buffer */
	return detFuncCoreProcessFinalSectionHelper(dFC);
}