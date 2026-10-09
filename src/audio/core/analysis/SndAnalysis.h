#pragma once

// Signal analysis helpers (audio/SndAnalysis.o): Numerical Recipes FFTs and
// the autocorrelation peak search behind PitchDetector. Parameter names are
// not in the reference map.

// In-place complex FFT of nn points; data is one-based. At 0xE32B0.
void four1(float* data, unsigned int nn, int isign);
// In-place real FFT of n samples; data is one-based. At 0xE34F0.
void realft(float* data, unsigned int n, int isign);
// Replaces n samples with their n / 2 + 1 magnitudes; data[0] holds the DC
// term. At 0xE3710.
void RealFT(float* data, int n);
// out[lag] = sum of x[i + lag] * x[i] over the first n / 2 samples, for each
// lag below n / 2. The flag is not read in this build. At 0xE37D0.
void ShiftedDotProduct(const float* x, int n, float* out, bool unused);
// Writes up to maxPeaks local maxima above threshold whose height times
// ratio exceeds the largest maximum, then -1 through peaks[maxPeaks]. At
// 0xE38F0.
void FindPeaks(float* x, int n, int* peaks, int maxPeaks, float threshold, float ratio);
// Whether a is within (a / b rounded) of a multiple of b. At 0xE39D0.
bool MultipleOf(int a, int b);
// Picks the period from a correlation and its running sum of squares, or 0
// when no lag is periodic enough. At 0xE3A00.
int FindCCPeak(const float* cc, const float* sos, int n, int minPeriod);
// Period from the spacing of the peaks FindPeaks returns, or 0. Not called
// in this build. At 0xE3CA0.
int FindIntPeriod(float* x, float* sos, int n, int* peaks, int minSpacing);
// Fractional refinement of an integer period found by FindCCPeak; 0 when it
// fails. At 0xE3F80.
float RefinePeriod2(const float* x, const float* sos, const float* cc, int n, int period);
// Earlier refinement by linear interpolation over 30 samples. Not called in
// this build. At 0xE4210.
float RefinePeriod(float* x, int n, int period);
// Adds the squared magnitude of bin i to den and its moment to num. At
// 0xE4610.
void AddPeakSqWeight(float& num, float& den, float* x, int i);
// Centroid of a spectral peak and its falling neighbours. At 0xE4640.
float FFTtoPeriod(float* x, int n, int peak);
// Peak position interpolated toward its larger neighbour. At 0xE4710.
float FFTtoPeriod2(float* x, int n, int peak);
// Fills n window weights: 0 rectangular, 1 triangular, 2 Hamming,
// 3 Blackman. At 0xE4780.
void MakeWindowFunc(float* window, int n, int type);
