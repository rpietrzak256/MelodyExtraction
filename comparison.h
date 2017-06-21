#include "sndfile.h"
#include "fftw3.h"
#include "pitchStrat.h"
#include "onsetStrat.h"

void PrintAudioMetadata(SF_INFO * file);
float* WindowFunction(int size);
struct Midi* ExtractMelody(float** input, SF_INFO info,
		int p_unpaddedSize, int p_winSize, int p_winInt, PitchStrategyFunc pitchStrategy,
		int o_unpaddedSize, int o_winSize, int o_winInt, OnsetStrategyFunc onsetStrategy,
		int hpsOvr, int verbose, char* prefix);
int ExtractPitch(float** input, float** pitches, SF_INFO info,
		int p_unpaddedSize, int p_winSize, int p_winInt, PitchStrategyFunc pitchStrategy,
		int hpsOvr, int verbose, char* prefix);
void ExtractOnset(float** input, SF_INFO info, int o_unpaddedSize, int o_winSize, 
                  int o_winInt, OnsetStrategyFunc onsetStrategy, int verbose);
int STFT_r2c(float** input, SF_INFO info, int unpaddedSize, int winSize, int interval, fftwf_complex** fft_data);
int STFTinverse_c2r(fftwf_complex** input, SF_INFO info, int winSize, int interval, float** output);
float* Magnitude(fftwf_complex* arr, int size);
void SaveAsWav(const double* audio, SF_INFO info, const char* path);
void SaveWeightsTxt(char* fileName, float** AudioData, int size, int dftBlocksize, int samplerate, int unpaddedSize, int winSize);