#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <assert.h>
#include <string.h>
#include "sndfile.h"
#include "fftw3.h"
#include "comparison.h"
#include "midi.h"
#include "detectionStrat.h"

const char* ERR_INVALID_FILE = "Audio file could not be opened for processing\n";
const char* ERR_FILE_NOT_MONO = "Input file must be Mono."
                          " Multi-channel audio currently not supported.\n";

void PrintAudioMetadata(SF_INFO * file)
{
	//this is only for printing information for debugging purposes
    printf("Frames:\t%ld\n", file->frames); 
    printf("Sample rate:\t%d\n", file->samplerate);
    printf("Channels: \t%d\n", file->channels);
    printf("Format: \t%d\n", file->format);
    printf("Sections: \t%d\n", file->sections);
    printf("Seekable: \t%d\n", file->seekable);
}

float* WindowFunction(int size)
{
	//uses Hamming Window
	//note: size = blocksize+1
	float* buffer = malloc(sizeof(float) * size);
	for(int i = 0; i < size; ++i) {
		buffer[i] =(float) (0.54-(0.46*cos(2*M_PI*i/(size-1.0))));
	}
	return buffer;
}

int ExtractMelody(char* inFile, char* outFile, int winSize, int winInt, int hpsOvr, int verbose, char* prefix, PitchDetectionStrategyFunc detectionStrategy)
{ 
	//reads in .wav
	SF_INFO info;
	SNDFILE * f = sf_open(inFile, SFM_READ, &info);
	if( !f ){
		printf("%s", ERR_INVALID_FILE);
		return -1;
	}
	if(info.channels != 1){
		printf("%s", ERR_FILE_NOT_MONO);
		sf_close( f );
		return -1;
	}
	
	if(verbose){
		PrintAudioMetadata(&info);
	}

	double* input = malloc( sizeof(double) * info.frames);
	sf_readf_double( f, input, info.frames );
	sf_close( f );

	fftw_complex* fftData = NULL;
	int size = STFT(&input, info, winSize, winInt, &fftData);
	int numBlocks = size/(winSize/2);
	if(verbose){
		printf("numblcks of STFT: %d\n", numBlocks);
		fflush(NULL);
	}

	double* spectrum = Magnitude(fftData, size);
	if(verbose){
		printf("Magnitude complete\n");
		fflush(NULL);
	}

	if (prefix !=NULL){
		// Here we save the original spectra
		char *spectraFile = malloc(sizeof(char) * (strlen(prefix)+14));
		strcpy(spectraFile,prefix);
		strcat(spectraFile,"_original.txt");
		SaveWeightsTxt(spectraFile, &spectrum, size, winSize/2, info.samplerate, winSize);
		free(spectraFile);
	}
	
	float* freq = detectionStrategy(&spectrum, size, winSize/2, hpsOvr,
					winSize, info.samplerate);

	if(verbose){
		printf("HPS complete\n");
		fflush(NULL);
	}

	int* melodyMidi = malloc(sizeof(float) * numBlocks);

	if (prefix !=NULL){
		// Here we save the processed spectra
		char *spectraFile = malloc(sizeof(char) * (strlen(prefix)+14));
		strcpy(spectraFile,prefix);
		strcat(spectraFile,"_weighted.txt");
		SaveWeightsTxt(spectraFile, &spectrum, size, winSize/2, info.samplerate, winSize);
		free(spectraFile);
	}
	
	//get midi note values of pitch in each bin
	for(int i = 0; i < numBlocks; ++i){
		if(verbose){
			printf("block%d:", i);
			fflush(NULL);
			printf("   freq: %.2f", freq[i]);
			fflush(NULL);
		}
		
		melodyMidi[i] = FrequencyToNote(freq[i]);
		if(verbose){
			printf("   midi: %d", melodyMidi[i]);
			fflush(NULL);
		}
		char* noteName = calloc(5, sizeof(char));
		NoteToName(melodyMidi[i], &noteName);
		if(verbose){
			printf("   name: %s \n", noteName);
			fflush(NULL);
		}
		free(noteName);
	}

	printf("printout complete\n");
	fflush(NULL);

	SaveMIDI(melodyMidi, numBlocks, outFile, verbose);

	free(melodyMidi);


	//double* output = NULL;
	//STFTinverse(&fftData, info, winSize, winInt, &output);
	//if(verbose){
	//	printf("STFTInverse complete\n");
	//}
	free(fftData);

	//SaveAsWav(output, info, outFile);

	/*
	 * if input is free before STFTinverse is called, result changes...
	 */
	free( input );
	//free( output );

	return info.frames;
}

//reads in .wav, returns FFT by reference through dft_data, returns size of dft_data
int STFT(double** input, SF_INFO info, int winSize, int interval, fftw_complex** dft_data)
{
    int i;
    int j;

    double* fftw_in = fftw_malloc( sizeof( double ) * winSize * info.channels );
    fftw_complex* fftw_out = fftw_malloc( sizeof( fftw_complex ) * winSize );

    fftw_plan plan  = fftw_plan_dft_r2c_1d( winSize, fftw_in, fftw_out, FFTW_MEASURE );

    float* window = WindowFunction(winSize+1);
 
    //malloc space for dft_data
	int numBlocks = (int)ceil((info.frames - winSize) / (double)interval) + 1;
	if(numBlocks < 1){
		numBlocks = 1;
	}
	//allocate winsize/2 for each block, taking the real component 
	//and dropping the complex component and nyquist frequency.
	int realWinSize = winSize/2;
    (*dft_data) = malloc( sizeof(fftw_complex) * numBlocks * realWinSize );
 
	int blockoffset;
    //run fft on each block
    for(i = 0; i < numBlocks; i++){

		// Copy the chunk into our buffer
		blockoffset = i*interval;
		for(j = 0; j < winSize; j++) {

			if(blockoffset + j < info.frames) {
				fftw_in[j] = (*input)[blockoffset + j] * window[j]; 
			} else {
				//reached end of input
				//Pad with 0 so fft is same size as other blocks
				fftw_in[j] = 0.0; 
			}
		}

		fftw_execute( plan );

		for (j = 0; j < realWinSize; j++) {
			(*dft_data)[i*realWinSize + j][0] = fftw_out[j][0];
			(*dft_data)[i*realWinSize + j][1] = fftw_out[j][1];
		}	
	}

	fftw_destroy_plan( plan );
	fftw_free( fftw_in );
	fftw_free( fftw_out );

	return numBlocks * realWinSize;
}

int STFTinverse(fftw_complex** input, SF_INFO info, int winSize, int interval, double** output)
{
	//length of input is numBlocks * (winSize/2 + 1)
    int i;
    int j;
	int inputoffset;
	int outputoffset;

    fftw_complex* fftw_in = fftw_malloc( sizeof( fftw_complex ) * winSize );
	double* fftw_out = fftw_malloc( sizeof( double ) * winSize * info.channels );

    fftw_plan plan  = fftw_plan_dft_c2r_1d( winSize, fftw_in, fftw_out, FFTW_MEASURE );

    //float* window = WindowFunction(winSize+1);

 	//malloc space for output
    (*output) = calloc( info.frames, sizeof(double));


	int numBlocks = (int)ceil((info.frames - winSize) / (double)interval) + 1;
	if(numBlocks < 1){
		numBlocks = 1;
	}

    //run fft on each block
    for(i = 0; i < numBlocks; i++){

		// Copy the chunk into our buffer
		inputoffset = i*(winSize/2 + 1);
		for(j = 0; j < winSize/2 + 1; j++) {
			fftw_in[j][0] = (*input)[inputoffset + j][0]; 
			fftw_in[j][1] = (*input)[inputoffset + j][1]; 
		}

		fftw_execute( plan );

		outputoffset = i*interval;

		for (j = 0; j < winSize; j++) {
			if(outputoffset + j < info.frames) {
				(*output)[outputoffset + j] += fftw_out[j] / (double)((winSize*winSize)/interval);
			}
		}	
	}

	fftw_destroy_plan( plan );
	fftw_free( fftw_in );
	fftw_free( fftw_out );

	return info.frames;
}

double* Magnitude(fftw_complex* arr, int size)
{
	int i;
	double* magArr = malloc( sizeof(double) * size);
	for(i = 0; i < size; i++){
		magArr[i] = (arr[i][0] * arr[i][0]) + (arr[i][1] * arr[i][1]);
	}
	return magArr;
}



void SaveAsWav(const double* audio, SF_INFO info, const char* path) {
	FILE* file = fopen(path, "wb");
	
	//Encode samples to 16 bit
	short* samples = (short*)malloc(info.frames* sizeof(short));
	unsigned int i;
	short max = -10000;
	int maxloc = 0;
	short min = 10000;
	int minloc = 0;
	for (i = 0; i < info.frames; ++i) {
		samples[i] = htole16(fmin(fmax(audio[i], -1), 1) * SHRT_MAX);
		if(samples[i] > max){
			max = samples[i];
			maxloc = i;
		}
		if(samples[i] < min){
			min = samples[i];
			minloc = i;
		}
	}

	printf("\tmax: %d at %d\tmin: %d at %d\n", max, maxloc, min, minloc);
	
	//Heaader chunk
	fprintf(file, "RIFF");
	unsigned int chunksize = htole32((info.frames * sizeof(short)) + 36);
	fwrite(&chunksize, sizeof(chunksize), 1, file);
	fprintf(file, "WAVE");
	
	//Format chunk
	fprintf(file, "fmt ");
	unsigned int fmtchunksize = htole32(16);
	fwrite(&fmtchunksize, sizeof(fmtchunksize), 1, file);
	unsigned short audioformat = htole16(1);
	fwrite(&audioformat, sizeof(audioformat), 1, file);
	unsigned short numchannels = htole16(1);
	fwrite(&numchannels, sizeof(numchannels), 1, file);
	unsigned int samplerate = htole32(info.samplerate);
	fwrite(&samplerate, sizeof(samplerate), 1, file);
	unsigned int byterate = htole32(samplerate * numchannels * sizeof(short));
	fwrite(&byterate, sizeof(byterate), 1, file);
	unsigned short blockalign = htole16(numchannels * sizeof(short));
	fwrite(&blockalign, sizeof(blockalign), 1, file);
	unsigned short bitspersample = htole16(sizeof(short) * CHAR_BIT);
	fwrite(&bitspersample, sizeof(bitspersample), 1, file);
	
	//Data chunk
	fprintf(file, "data");
	unsigned int datachunksize = htole32(info.frames * sizeof(short));
	fwrite(&datachunksize, sizeof(datachunksize), 1, file);
	fwrite(samples, sizeof(short), info.frames, file);
	
	//Free encoded samples
	free(samples);
	
	//Close the file
	fclose(file);
}

void SaveWeightsTxt(char* fileName, double** AudioData, int size, int dftBlocksize, int samplerate, int winSize){
        // Saves the weights of each frequency bin to a text file
        FILE *fp;
	int blockstart, i;
	fp = fopen(fileName,"w");

	fprintf(fp, "#Window Size:\t%d\n", winSize);
	fprintf(fp, "#Sample Rate:\t%d\n", samplerate);
	// outer loop iterates over blocks
	for(blockstart = 0; blockstart < (size - dftBlocksize); blockstart += dftBlocksize){
		//iterate over elements of a block
		for(i = 0; i < dftBlocksize; ++i){
			fprintf(fp, "%e\t", (*AudioData)[blockstart + i]);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
}
