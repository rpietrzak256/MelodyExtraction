
double* BaNa(double **AudioData, int size, int dftBlocksize, int p,
	     double f0Min, double f0Max, double* frequencies);
double* BaNaMusic(double **AudioData, int size, int dftBlocksize, int p,
		  double f0Min, double f0Max, double* frequencies);
void BaNaPreprocessing(double **AudioData, int size, int dftBlocksize, int p,
		       double f0Min, double f0Max, double* frequencies);
void BaNaFindCandidates(double **AudioData,int size,int dftBlocksize, int p,
			double f0Min, double f0Max, int first,
			struct candidateList **windowCandidates,
			double xi, double* frequencies);