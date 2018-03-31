#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include <limits.h>
#include "../src/sigOpt.h"
#include "../src/simpleDetFunc.h"
#include "doubleArrayTesting.h"

bufferIndex *buff0Ind4;

void setup_buff0Ind4(void){
	buff0Ind4 = bufferIndexCreate(0,4,6);
}

void teardown_buff0Ind4(void){
	bufferIndexDestroy(buff0Ind4);
}

START_TEST(test_bufferIndexCreate)
{
	ck_assert_int_eq(bufferIndexGetBufferNum(buff0Ind4),0);
	ck_assert_int_eq(bufferIndexGetIndex(buff0Ind4),4);
	ck_assert_int_eq(bufferIndexGetBufferLength(buff0Ind4),6);	
}
END_TEST


START_TEST(test_bufferIndexIncrement)
{
	int temp;
	temp = bufferIndexIncrement(buff0Ind4);
	ck_assert_int_eq(temp,1);
	ck_assert_int_eq(bufferIndexGetBufferNum(buff0Ind4),0);
	ck_assert_int_eq(bufferIndexGetIndex(buff0Ind4),5);
	temp = bufferIndexIncrement(buff0Ind4);
	ck_assert_int_eq(temp,1);
	ck_assert_int_eq(bufferIndexGetBufferNum(buff0Ind4),1);
	ck_assert_int_eq(bufferIndexGetIndex(buff0Ind4),0);
}
END_TEST

START_TEST(test_bufferIndexAdvanceBuffer)
{
	int temp;
	temp = bufferIndexAdvanceBuffer(buff0Ind4);
	ck_assert_int_eq(temp,1);
	ck_assert_int_eq(bufferIndexGetBufferNum(buff0Ind4),-1);
	
}
END_TEST

START_TEST(test_bufferIndexAddScalarIndex)
{
	bufferIndex *bI;
	// same bufferNum
	bI = bufferIndexAddScalarIndex(buff0Ind4, 1);
	ck_assert_ptr_nonnull(bI);
	ck_assert_int_eq(bufferIndexGetBufferNum(bI),0);
	ck_assert_int_eq(bufferIndexGetIndex(bI),5);
	bufferIndexDestroy(bI);

	// bufferNum 1 Index 0
	bI = bufferIndexAddScalarIndex(buff0Ind4, 2);
	ck_assert_ptr_nonnull(bI);
	ck_assert_int_eq(bufferIndexGetBufferNum(bI),1);
	ck_assert_int_eq(bufferIndexGetIndex(bI),0);
	bufferIndexDestroy(bI);

	// bufferNum2 Index 3
	bI = bufferIndexAddScalarIndex(buff0Ind4, 11);
	ck_assert_ptr_nonnull(bI);
	ck_assert_int_eq(bufferIndexGetBufferNum(bI),2);
	ck_assert_int_eq(bufferIndexGetIndex(bI),3);
	bufferIndexDestroy(bI);

	// failure - neg num
	bI = bufferIndexAddScalarIndex(buff0Ind4, -1);
	ck_assert_ptr_null(bI);
}
END_TEST

// test Case for comparison
// test Case for adding indices


bufferIndex *buff0Ind0;
bufferIndex *buff0Ind5;
bufferIndex *buffNeg1Ind2;
bufferIndex *buff1Ind3;
bufferIndex *buff0Ind4DiffLength;

void setup_comparisons(void){
	setup_buff0Ind4();
	buff0Ind0 = bufferIndexCreate(0,0,6);
	buff0Ind5 = bufferIndexCreate(0,5,6);
	buffNeg1Ind2 = bufferIndexCreate(-1,2,6);
	buff1Ind3 = bufferIndexCreate(0,5,6);
	buff0Ind4DiffLength = bufferIndexCreate(0,4,7);
}

void teardown_comparisons(void){
	teardown_buff0Ind4();
	bufferIndexDestroy(buff0Ind0);
	bufferIndexDestroy(buff0Ind5);
	bufferIndexDestroy(buffNeg1Ind2);
	bufferIndexDestroy(buff1Ind3);
	bufferIndexDestroy(buff0Ind4DiffLength);
}

START_TEST(test_bufferIndexEq)
{
	bufferIndex* bI= bufferIndexCreate(0,4,6);
	ck_assert_int_eq(bufferIndexEq(buff0Ind4, bI), 1);
	ck_assert_int_eq(bufferIndexEq(buff0Ind0, bI), 0);
	ck_assert_int_eq(bufferIndexEq(buff0Ind5, bI), 0);
	ck_assert_int_eq(bufferIndexEq(buffNeg1Ind2, bI), 0);
	ck_assert_int_eq(bufferIndexEq(buff1Ind3, bI), 0);
	ck_assert_int_eq(bufferIndexEq(buff0Ind4DiffLength, bI), 0);
	bufferIndexDestroy(bI);
}
END_TEST

START_TEST(test_bufferIndexNe)
{
	bufferIndex* bI= bufferIndexCreate(0,4,6);
	ck_assert_int_eq(bufferIndexNe(buff0Ind4, bI), 0);
	ck_assert_int_eq(bufferIndexNe(buff0Ind0, bI), 1);
	ck_assert_int_eq(bufferIndexNe(buff0Ind5, bI), 1);
	ck_assert_int_eq(bufferIndexNe(buffNeg1Ind2, bI), 1);
	ck_assert_int_eq(bufferIndexNe(buff1Ind3, bI), 1);
	ck_assert_int_eq(bufferIndexNe(buff0Ind4DiffLength, bI), 1);
	bufferIndexDestroy(bI);
}
END_TEST

START_TEST(test_bufferIndexGt)
{
	bufferIndex* bI= bufferIndexCreate(0,4,6);
	ck_assert_int_eq(bufferIndexGt(buff0Ind4, bI), 0);
	ck_assert_int_eq(bufferIndexGt(buff0Ind0, bI), 0);
	ck_assert_int_eq(bufferIndexGt(buff0Ind5, bI), 1);
	ck_assert_int_eq(bufferIndexGt(buffNeg1Ind2, bI), 0);
	ck_assert_int_eq(bufferIndexGt(buff1Ind3, bI), 1);
	ck_assert_int_eq(bufferIndexGt(buff0Ind4DiffLength, bI), 0);
	bufferIndexDestroy(bI);
}
END_TEST

START_TEST(test_bufferIndexGe)
{
	bufferIndex* bI= bufferIndexCreate(0,4,6);
	ck_assert_int_eq(bufferIndexGe(buff0Ind4, bI), 1);
	ck_assert_int_eq(bufferIndexGe(buff0Ind0, bI), 0);
	ck_assert_int_eq(bufferIndexGe(buff0Ind5, bI), 1);
	ck_assert_int_eq(bufferIndexGe(buffNeg1Ind2, bI), 0);
	ck_assert_int_eq(bufferIndexGe(buff1Ind3, bI), 1);
	ck_assert_int_eq(bufferIndexGe(buff0Ind4DiffLength, bI), 0);
	bufferIndexDestroy(bI);
}
END_TEST

START_TEST(test_bufferIndexLt)
{
	bufferIndex* bI= bufferIndexCreate(0,4,6);
	ck_assert_int_eq(bufferIndexLt(buff0Ind4, bI), 0);
	ck_assert_int_eq(bufferIndexLt(buff0Ind0, bI), 1);
	ck_assert_int_eq(bufferIndexLt(buff0Ind5, bI), 0);
	ck_assert_int_eq(bufferIndexLt(buffNeg1Ind2, bI), 1);
	ck_assert_int_eq(bufferIndexLt(buff1Ind3, bI), 0);
	ck_assert_int_eq(bufferIndexLt(buff0Ind4DiffLength, bI), 0);
	bufferIndexDestroy(bI);
}
END_TEST

START_TEST(test_bufferIndexLe)
{
	bufferIndex* bI= bufferIndexCreate(0,4,6);
	ck_assert_int_eq(bufferIndexLe(buff0Ind4, bI), 1);
	ck_assert_int_eq(bufferIndexLe(buff0Ind0, bI), 1);
	ck_assert_int_eq(bufferIndexLe(buff0Ind5, bI), 0);
	ck_assert_int_eq(bufferIndexLe(buffNeg1Ind2, bI), 1);
	ck_assert_int_eq(bufferIndexLe(buff1Ind3, bI), 0);
	ck_assert_int_eq(bufferIndexLe(buff0Ind4DiffLength, bI), 0);
	bufferIndexDestroy(bI);
}
END_TEST

START_TEST(test_bufferIndexCreate_NegIndex)
{
	bufferIndex *bI = bufferIndexCreate(0,-1,6);
	ck_assert_msg(bI == NULL,
		      "NULL should be returned as result of attempt to create "
		      "a bufferIndex with a negative Index.");
}
END_TEST

START_TEST(test_bufferIndexCreate_NegBufferSize)
{
	bufferIndex *bI = bufferIndexCreate(0, 5, -1);
	ck_assert_msg(bI == NULL,
		      "NULL should be returned as result of attempt to create "
		      "a buffer length with a negative value.");
}
END_TEST

START_TEST(test_bufferIndexCreate_ZeroBufferSize)
{
	bufferIndex *bI = bufferIndexCreate(0, 0, 0);
	ck_assert_msg(bI == NULL,
		      "NULL should be returned as result of attempt to create "
		      "a buffer length with zero length.");
}
END_TEST

START_TEST(test_bufferIndexCreate_IndexBufferSize)
{
	bufferIndex *bI = bufferIndexCreate(0,6,6);
	ck_assert_msg(bI == NULL,
		      "NULL should be returned as result of attempt to create "
		      "a bufferIndex with an index equal to the buffer size.");
	bI = bufferIndexCreate(0,7,6);
	ck_assert_msg(bI == NULL,
		      "NULL should be returned as result of attempt to create "
		      "a bufferIndex with an index greater than the buffer "
		      "size.");
}
END_TEST



START_TEST(test_sigOptCreate)
{
	// using a buffer size of ~70 ms for samplerate of 11025 Hz
	// since 55 samples is ~ 5ms. This means the window size is 770
	// samples
	sigOpt *sO =sigOptCreate(770,55,22,4,1.06);
	ck_assert_ptr_nonnull(sO);
	ck_assert_int_eq(sigOptGetBufferLength(sO),462);
	ck_assert_int_eq(sigOptGetSigmasPerBuffer(sO),8);
	sigOptDestroy(sO);
}
END_TEST


START_TEST(test_sigOptSetTerminationIndex)
{
	int temp;
	sigOpt *sO =sigOptCreate(770,55,22,4,1.06);
	ck_assert_ptr_nonnull(sO);
	temp = sigOptSetTerminationIndex(sO,-1);
	ck_assert_int_ne(temp,1);
	temp = sigOptSetTerminationIndex(sO,462);
	ck_assert_int_ne(temp,1);
	temp = sigOptSetTerminationIndex(sO,463);
	ck_assert_int_ne(temp,1);
	temp = sigOptSetTerminationIndex(sO,4);
	ck_assert_int_eq(temp,1);
	temp = sigOptSetTerminationIndex(sO,8);
	ck_assert_int_ne(temp,1);
	sigOptDestroy(sO);
}
END_TEST


START_TEST(test_sigOptCreate_negWinSize)
{
	sigOpt *sO =sigOptCreate(-770,55,22,4,1.06);
	ck_assert_ptr_null(sO);
}
END_TEST

START_TEST(test_sigOptCreate_zeroWinSize)
{
	sigOpt *sO =sigOptCreate(0,55,22,4,1.06);
	ck_assert_ptr_null(sO);
}
END_TEST

START_TEST(test_sigOptCreate_negHopsize)
{
	sigOpt *sO =sigOptCreate(770,-55,22,4,1.06);
	ck_assert_ptr_null(sO);
}
END_TEST

START_TEST(test_sigOptCreate_zeroHopsize)
{
	sigOpt *sO =sigOptCreate(770,0,22,4,1.06);
	ck_assert_ptr_null(sO);
}
END_TEST

START_TEST(test_sigOptCreate_negScaleFactor)
{
	sigOpt *sO =sigOptCreate(770,55,22,4,-1.06);
	ck_assert_ptr_null(sO);
}
END_TEST

START_TEST(test_sigOptCreate_zeroScaleFactor)
{
	sigOpt *sO =sigOptCreate(770,0,22,4,0);
	ck_assert_ptr_null(sO);
}
END_TEST

START_TEST(test_sigOptCreate_negInitialOffset)
{
	sigOpt *sO =sigOptCreate(770,55,-22,4,1.06);
	ck_assert_ptr_null(sO);
}
END_TEST

START_TEST(test_sigOptCreate_zeroInitialOffset)
{
	sigOpt *sO =sigOptCreate(770,55,0,4,1.06);
	ck_assert_ptr_nonnull(sO);
	sigOptDestroy(sO);
}
END_TEST

START_TEST(test_sigOptCreate_InitialOffsetWinSize)
{
	sigOpt *sO =sigOptCreate(770,55,770,4,1.06);
	ck_assert_msg(sO == NULL,
		      "NULL should be returned as result of attempt to create "
		      "a sigOpt with a startIndex>= winSize.");
}
END_TEST

START_TEST(test_sigOptCreate_negChannel)
{
	sigOpt *sO =sigOptCreate(770,55,22,-4,1.06);
	ck_assert_ptr_null(sO);
}
END_TEST

START_TEST(test_sigOptCreate_zeroChannel)
{
	sigOpt *sO =sigOptCreate(770,55,22,0,1.06);
	ck_assert_ptr_null(sO);
}
END_TEST


/* The next set of tests will actually demonstrate sigOpt in action. */
double *dblrollSigma(int initialOffset, int hopsize, float scaleFactor,
		     int winSize, int dataLength, int numWindows,
		     float *input, int testFunc)
{
	float *fltSigma= malloc(sizeof(float)*numWindows);
	if (testFunc == 1){
		int temp = sigOptFullRollSigma(initialOffset, hopsize,
					       scaleFactor, winSize, dataLength,
					       numWindows, input, &fltSigma);
		if (temp < 0){
			free(fltSigma);
			return NULL;
		}
	} else {
		rollSigma(initialOffset, hopsize, scaleFactor, winSize,
			  dataLength, numWindows, input, &fltSigma);
	}
	double *result = NULL;
	float_to_double_array(fltSigma, numWindows, &result);
	free(fltSigma);
	return result;
}

float* fltInput;
int inputLength;

void setup_sigOptRunExamples(void){
	double *original_input;
	inputLength = readDoubleArray(("tests/test_files/roll_sigma/"
				       "roll_sigma_sample_input"),
				      isLittleEndian(), &original_input);
	//inputLength is 51
	double_to_float_array(original_input, inputLength, &fltInput);
	free(original_input);
}

void teardown_sigOptRunExamples(void){
	free(fltInput);
}


START_TEST(test_sigOptFullRollSigma_Simple)
{
	int numWindows = 10; // not sure if this is a good value. May need to
	                     // come back and fix this.
	int initialOffset = 1;
	int winSize = 15;
	int hopsize = 5;

	double *reference = dblrollSigma(initialOffset, hopsize, 1.06,
					 winSize, inputLength, numWindows,
					 fltInput, 0); //rollSigma

	// this time we will run sigOpt manually for debugging.
	sigOpt *sO = sigOptCreate(winSize, hopsize, initialOffset, 1,
				  1.06);
	ck_assert_ptr_nonnull(sO);
	int bufferLength = sigOptGetBufferLength(sO);
	ck_assert_int_eq(bufferLength,11);
	int sigPerBuffer = sigOptGetSigmasPerBuffer(sO);
	ck_assert_int_eq(sigPerBuffer,2);

	float *centralBuffer = fltInput;
	int intermed_result = sigOptSetup(sO, 0, centralBuffer);
	ck_assert_int_eq(intermed_result,1);
	float *leadingBuffer = fltInput + bufferLength;
	float *trailingBuffer = NULL;
	printf("%f,%f\n",centralBuffer[bufferLength],leadingBuffer[0]);
	float sig = sigOptAdvanceWindow(sO, NULL,
					centralBuffer,leadingBuffer,0);
	printf("%f\n",sig);
	printf("%lf\n",reference[0]);
	ck_assert_float_eq(sig,(float)(reference[0]));
	fflush(stdout);

	// now the window is centered on index 6
	// the left edge extends to 0 (it theoretically includes index -1)
	// the stop index is index 14, or the 4 th index of leadingBuffer
	sig = sigOptAdvanceWindow(sO, NULL, centralBuffer, leadingBuffer,
				  0);
	ck_assert_float_eq(sig,(float)(reference[1]));

	// now for advancement across buffers
	trailingBuffer = centralBuffer;
	centralBuffer = leadingBuffer;
	leadingBuffer = fltInput + 2*bufferLength;
	sigOptAdvanceBuffer(sO);
	// now the window is centered at index 11 (the first index of the
	// central/ second buffer)
	// at this point the left edge should now include index 4 and the right
	// edge should stop at index 19 (i.e. index 19 should not be included).
	// In other words the right edge should stop at index 9 of the second
	// (central) buffer
	sig = sigOptAdvanceWindow(sO, trailingBuffer, centralBuffer,
				  leadingBuffer, 0);
	ck_assert_float_eq(sig,(float)(reference[2]));
	
	
	

	sigOptDestroy(sO);
	ck_abort();
	// I would like to run this after we solve our problems
	double *result = dblrollSigma(initialOffset, hopsize, 1.06,
				      winSize, inputLength, numWindows,
				      fltInput, 1); //sigOptRollSigma
	ck_assert_ptr_nonnull(result);
	
	compareArrayEntries(reference, result, numWindows, 1.e-5, 1, 1.e-5);

	free(reference);
	free(result);
}
END_TEST

Suite *sigOpt_suite(void)
{
	Suite *s;
	TCase *tc_sOcore;
	TCase *tc_sOlimits;
	TCase *tc_sOrun;
	TCase *tc_bIcore;
	TCase *tc_bIlimits;
	TCase *tc_bIcomp;

	s = suite_create("sigOpt");

	/* Core test case */
	tc_sOcore = tcase_create("sigOpt Core");
	tcase_add_test(tc_sOcore,test_sigOptCreate);
	tcase_add_test(tc_sOcore,test_sigOptSetTerminationIndex);
	suite_add_tcase(s, tc_sOcore);

	tc_sOlimits = tcase_create("sigOpt Limits");
	tcase_add_test(tc_sOlimits,test_sigOptCreate_negWinSize);
	tcase_add_test(tc_sOlimits,test_sigOptCreate_zeroWinSize);
	tcase_add_test(tc_sOlimits,test_sigOptCreate_negHopsize);
	tcase_add_test(tc_sOlimits,test_sigOptCreate_zeroHopsize);
	tcase_add_test(tc_sOlimits,test_sigOptCreate_negScaleFactor);
	tcase_add_test(tc_sOlimits,test_sigOptCreate_zeroScaleFactor);
	tcase_add_test(tc_sOlimits,test_sigOptCreate_negInitialOffset);
	tcase_add_test(tc_sOlimits,test_sigOptCreate_zeroInitialOffset);
	tcase_add_test(tc_sOlimits,test_sigOptCreate_InitialOffsetWinSize);
	tcase_add_test(tc_sOlimits,test_sigOptCreate_negChannel);
	tcase_add_test(tc_sOlimits,test_sigOptCreate_zeroChannel);
	suite_add_tcase(s, tc_sOlimits);

	tc_sOrun = tcase_create("sigOpt Run Examples");
	if (isLittleEndian() == 1){
		tcase_add_checked_fixture(tc_sOrun, setup_sigOptRunExamples,
					  teardown_sigOptRunExamples);
		tcase_add_test(tc_sOrun, test_sigOptFullRollSigma_Simple);
	} else {
		printf("Cannot add rollSigma tests because the tests are not \n"
		       "presently equipped to run on Big Endian machines\n");
	}
	suite_add_tcase(s,tc_sOrun);

	tc_bIcore = tcase_create("bufferIndex Core");
	tcase_add_checked_fixture(tc_bIcore, setup_buff0Ind4,
				  teardown_buff0Ind4);
	tcase_add_test(tc_bIcore,test_bufferIndexCreate);
	tcase_add_test(tc_bIcore,test_bufferIndexIncrement);
	tcase_add_test(tc_bIcore,test_bufferIndexAdvanceBuffer);
	tcase_add_test(tc_bIcore,test_bufferIndexAddScalarIndex);
	suite_add_tcase(s, tc_bIcore);

	tc_bIcomp = tcase_create("bufferIndex Comparisons");
	tcase_add_checked_fixture(tc_bIcomp, setup_comparisons,
				  teardown_comparisons);
	tcase_add_test(tc_bIcomp,test_bufferIndexEq);
	tcase_add_test(tc_bIcomp,test_bufferIndexNe);
	tcase_add_test(tc_bIcomp,test_bufferIndexGt);
	tcase_add_test(tc_bIcomp,test_bufferIndexGe);
	tcase_add_test(tc_bIcomp,test_bufferIndexLt);
	tcase_add_test(tc_bIcomp,test_bufferIndexLe);
	suite_add_tcase(s,tc_bIcomp);

	tc_bIlimits = tcase_create("bufferIndex Limits");
	tcase_add_test(tc_bIlimits,test_bufferIndexCreate_NegIndex);
	tcase_add_test(tc_bIlimits,test_bufferIndexCreate_NegBufferSize);
	tcase_add_test(tc_bIlimits,test_bufferIndexCreate_ZeroBufferSize);
	tcase_add_test(tc_bIlimits,test_bufferIndexCreate_IndexBufferSize);
	suite_add_tcase(s, tc_bIlimits);

	
	return s;
}

int main(void)
{
	int number_failed;
	Suite *s;
	SRunner *sr;
	s = sigOpt_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	if (number_failed == 0){
		return EXIT_SUCCESS;
	} else {
		return EXIT_FAILURE;
	}
};