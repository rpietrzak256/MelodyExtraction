int isMidiNote(int note);
void NoteToName(int n, char** name);
int FrequencyToNote(double freq);
double log2(double x);
void SaveMIDI(int* noteArr, int size, char* path, int verbose);
void AddHeader(FILE** f, short format, short tracks, short division);
void AddTrack(FILE** f, unsigned char* track, int len);
int MakeTrack(unsigned char** track, int* noteArr, int size);
void AppendInt(unsigned char** c, int num);
void AppendShort(unsigned char** c, short num);
int IntToVLQ(unsigned int num, unsigned char** VLQ);
unsigned char* MessageNoteOn(int pitch, int velocity);
unsigned char* MessageNoteOff(int pitch, int velocity);