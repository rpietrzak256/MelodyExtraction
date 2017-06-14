#ifndef LISTS_H
#define LISTS_H

struct orderedList{
	double *array;
	int max_length;
	int length;
};

struct candidate{
  float frequency;
  int confidence;
  float cost;
  int indexLowestCost;
};

struct candidateList{
  struct candidate *array;
  int max_length;
  int length;
};

#endif /*LISTS_H*/

int bisectLeftD(double* l, double value, int low, int high);
int bisectLeft(float* l, float value, int low, int high);
struct orderedList orderedListCreate(int max_length);
void orderedListDestroy(struct orderedList list);
double orderedListGet(struct orderedList list, int index);
int bisectInsertionSearch(struct orderedList list, double value);
void orderedListInsert(struct orderedList *list, double value);
void orderedListDeleteEntries(struct orderedList *list, int start, int stop);
void orderedListPrint(struct orderedList list);

struct candidateList* candidateListCreate(int max_length);
void candidateListDestroy(struct candidateList* list);
struct candidate candidateListGet(struct candidateList list, int index);
void candidateListAdd(struct candidateList *list, float frequency,
		      int confidence);
void candidateListResize(struct candidateList *list);
void candidateListAdjustCost(struct candidateList *list, int index,
			     float cost, int indexLowestCost);
void candidateListPrintFreq(struct candidateList list);
void candidateListPrintConfidence(struct candidateList list);
