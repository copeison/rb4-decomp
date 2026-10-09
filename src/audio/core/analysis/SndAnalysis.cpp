#include "audio/core/analysis/SndAnalysis.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

// Reconstructed from eboot.elf at 0xE32B0.
void four1(float* data, unsigned int nn, int isign) {
    unsigned long n = static_cast<unsigned long>(nn) << 1;
    unsigned long j = 1;
    for (unsigned long i = 1; i < n; i += 2) {
        if (j > i) {
            std::swap(data[j], data[i]);
            std::swap(data[j + 1], data[i + 1]);
        }
        unsigned long m = nn;
        while (m >= 2 && j > m) {
            j -= m;
            m >>= 1;
        }
        j += m;
    }
    unsigned long mmax = 2;
    while (n > mmax) {
        unsigned long istep = mmax << 1;
        float theta = isign * (6.28318530717959f / mmax);
        float wtemp = sinf(0.5f * theta);
        float wpr = -2.0f * wtemp * wtemp;
        float wpi = sinf(theta);
        float wr = 1.0f;
        float wi = 0.0f;
        for (unsigned long m = 1; m < mmax; m += 2) {
            for (unsigned long i = m; i <= n; i += istep) {
                j = i + mmax;
                float tempr = wr * data[j] - wi * data[j + 1];
                float tempi = wr * data[j + 1] + wi * data[j];
                data[j] = data[i] - tempr;
                data[j + 1] = data[i + 1] - tempi;
                data[i] += tempr;
                data[i + 1] += tempi;
            }
            wtemp = wr;
            wr = wr * wpr - wi * wpi + wr;
            wi = wi * wpr + wtemp * wpi + wi;
        }
        mmax = istep;
    }
}

// Reconstructed from eboot.elf at 0xE34F0.
void realft(float* data, unsigned int n, int isign) {
    float c1 = 0.5f;
    float c2;
    float theta = 3.141592653589793f / static_cast<float>(n >> 1);
    if (isign == 1) {
        c2 = -0.5f;
        four1(data, n >> 1, 1);
    } else {
        c2 = 0.5f;
        theta = -theta;
    }
    float wtemp = sinf(0.5f * theta);
    float wpr = -2.0f * wtemp * wtemp;
    float wpi = sinf(theta);
    float wr = 1.0f + wpr;
    float wi = wpi;
    unsigned long np3 = n + 3;
    for (unsigned long i = 2; i <= (n >> 2); ++i) {
        unsigned long i1 = i + i - 1;
        unsigned long i2 = 1 + i1;
        unsigned long i3 = np3 - i2;
        unsigned long i4 = 1 + i3;
        float h1r = c1 * (data[i1] + data[i3]);
        float h1i = c1 * (data[i2] - data[i4]);
        float h2r = -c2 * (data[i2] + data[i4]);
        float h2i = c2 * (data[i1] - data[i3]);
        data[i1] = h1r + wr * h2r - wi * h2i;
        data[i2] = h1i + wr * h2i + wi * h2r;
        data[i3] = h1r - wr * h2r + wi * h2i;
        data[i4] = -h1i + wr * h2i + wi * h2r;
        wtemp = wr;
        wr = wr * wpr - wi * wpi + wr;
        wi = wi * wpr + wtemp * wpi + wi;
    }
    float h1r = data[1];
    if (isign == 1) {
        data[1] = h1r + data[2];
        data[2] = h1r - data[2];
    } else {
        data[1] = c1 * (h1r + data[2]);
        data[2] = c1 * (h1r - data[2]);
        four1(data, n >> 1, -1);
    }
}

// Reconstructed from eboot.elf at 0xE3710.
void RealFT(float* data, int n) {
    realft(data - 1, n, 1);
    data[0] = fabsf(data[0]);
    float nyquist = fabsf(data[1]);
    int half = n / 2;
    for (int i = 1; i < half; ++i) {
        float re = data[2 * i];
        float im = data[2 * i + 1];
        data[i] = sqrtf(re * re + im * im);
    }
    data[half] = nyquist;
}

// Reconstructed from eboot.elf at 0xE37D0.
void ShiftedDotProduct(const float* x, int n, float* out, bool) {
    if (n < 2) {
        return;
    }
    int half = n / 2;
    for (int lag = 0; lag < half; ++lag) {
        float sum = 0.0f;
        for (int i = 0; i < half; ++i) {
            sum += x[i + lag] * x[i];
        }
        out[lag] = sum;
    }
}

namespace {

// Candidate peaks gathered by FindPeaks, at 0x19E2710. Name not in the
// reference map.
int sPeakCandidates[100];

}  // namespace

// Reconstructed from eboot.elf at 0xE38F0.
void FindPeaks(float* x, int n, int* peaks, int maxPeaks, float threshold, float ratio) {
    int count = 0;
    if (n - 1 >= 2) {
        float highest = 0.0f;
        int numCandidates = 0;
        for (int i = 1; i < n - 1 && numCandidates < 100; ++i) {
            float value = x[i];
            if (value > threshold && value > x[i - 1] && value > x[i + 1]) {
                highest = std::max(value, highest);
                sPeakCandidates[numCandidates++] = i;
            }
        }
        for (int i = 0; i < numCandidates && count < maxPeaks; ++i) {
            int peak = sPeakCandidates[i];
            if (ratio * x[peak] > highest) {
                peaks[count++] = peak;
            }
        }
    }
    for (int i = count; i <= maxPeaks; ++i) {
        peaks[i] = -1;
    }
}

// Reconstructed from eboot.elf at 0xE39D0.
bool MultipleOf(int a, int b) {
    int multiple = static_cast<int>(static_cast<float>(a) / static_cast<float>(b) + 0.5f);
    return std::abs(multiple * b - a) <= multiple;
}

// Reconstructed from eboot.elf at 0xE3A00. A lag qualifies when it is a
// local maximum whose correlation, normalized by the energies of the two
// windows, exceeds 0.75. Each candidate is weighted by its period to the
// power log2(1 / 1.4), favouring shorter periods.
int FindCCPeak(const float* cc, const float* sos, int n, int minPeriod) {
    float values[10];
    int periods[10];
    float weighted[10];
    int count = 0;
    float highest = 0.0f;
    int half = n / 2 - 1;
    for (int i = minPeriod; i < half; ++i) {
        float value = cc[i];
        if (value > cc[i - 1] && value > cc[i + 1]) {
            float energy = (sos[i + half] - sos[i - 1]) * sos[half];
            if (energy != 0.0f) {
                float normalized = value / sqrtf(energy);
                if (normalized > 0.75f) {
                    highest = std::max(highest, normalized);
                    values[count] = normalized;
                    periods[count] = i;
                    if (count++ > 8) {
                        break;
                    }
                }
            }
        }
    }
    for (int i = 0; i < count; ++i) {
        // Evaluates to -0.4854. Name not in the reference map.
        static float sPeriodWeightExponent = logf(1.0f / 1.4f) / logf(2.0f);
        weighted[i] = powf(static_cast<float>(periods[i]), sPeriodWeightExponent) * values[i];
    }
    int period = periods[std::max_element(weighted, weighted + count) - weighted];
    if (period <= 10 && highest < 0.99f && count < 9) {
        return 0;
    }
    if (highest < 0.9f || count == 0) {
        return 0;
    }
    return period;
}

// Reconstructed from eboot.elf at 0xE3CA0.
int FindIntPeriod(float* x, float* sos, int n, int* peaks, int minSpacing) {
    if (peaks[0] == -1 || peaks[1] == -1) {
        return 0;
    }
    int best = peaks[0];
    for (int i = 1; peaks[i] != -1; ++i) {
        if (x[peaks[i]] > x[best]) {
            best = peaks[i];
        }
    }
    int spacings[10];
    int count = 0;
    for (int i = 0; peaks[i] != -1; ++i) {
        int spacing = std::abs(best - peaks[i]);
        if (spacing > minSpacing) {
            spacings[count++] = spacing;
        }
        if (count > 9) {
            break;
        }
    }
    std::sort(spacings, spacings + count);
    int previous = 0;
    for (int i = 0; i < count; ++i) {
        int period = spacings[i];
        if (period == previous || period == previous + 1) {
            previous = period;
            continue;
        }
        previous = period;
        int length = std::min(n - period, n / 2);
        float sum = 0.0f;
        for (int j = 0; j < length - 1; ++j) {
            sum += x[period + j] * x[j];
        }
        float energy = (sos[period + length - 2] - sos[period - 1]) * sos[length - 2];
        if (sum / sqrtf(energy) > 0.85f) {
            return period;
        }
    }
    return 0;
}

// Reconstructed from eboot.elf at 0xE3F80. Fits the correlation between
// lags period and period + 1, moving the period at most twice; rejects
// offsets beyond 3 samples.
float RefinePeriod2(const float* x, const float* sos, const float* cc, int n, int period) {
    float offset = 0.0f;
    if (period > 0) {
        int half = n / 2;
        int steps = 0;
        for (;;) {
            float cross = 0.0f;
            for (int i = 0; i < half; ++i) {
                cross += x[period + 1 + i] * x[period + i];
            }
            float energy = sos[period + half - 1] - sos[period - 1];
            offset = (energy - cc[period] + cc[period + 1] - cross)
                / (energy + sos[period + half] - sos[period] - 2.0f * cross);
            if (offset > 1.0f) {
                ++period;
            } else if (offset >= 0.0f) {
                break;
            } else {
                --period;
            }
            if (++steps > 1 || period <= 0) {
                break;
            }
        }
    }
    if (!(period + offset > 0.0f) || fabsf(offset) > 3.0f) {
        period = 0;
        offset = 0.0f;
    }
    return period + offset;
}

// Reconstructed from eboot.elf at 0xE4210. Steps the period while the
// interpolation falls outside [-0.01, 1.01], giving up after eight tries.
float RefinePeriod(float* x, int, int period) {
    if (period == 0) {
        return 0.0f;
    }
    int direction = 0;
    float fraction;
    for (int tries = 1;; ++tries) {
        if (tries > 8) {
            return 0.0f;
        }
        float num = 0.0f;
        float den = 0.0f;
        for (int i = 0; i < 30; ++i) {
            float step = x[period + i] - x[period + i + 1];
            num += (x[i] - x[period + i + 1]) * step;
            den += step * step;
        }
        fraction = num / den;
        if (fraction > 1.01) {
            if (direction == 1) {
                break;
            }
            direction = -1;
            --period;
        } else if (fraction >= -0.01 || direction == -1) {
            break;
        } else {
            direction = 1;
            ++period;
        }
    }
    return period + (1.0f - fraction);
}

// Reconstructed from eboot.elf at 0xE4610.
void AddPeakSqWeight(float& num, float& den, float* x, int i) {
    num += x[i] * x[i] * i;
    den += x[i] * x[i];
}

// Reconstructed from eboot.elf at 0xE4640.
float FFTtoPeriod(float* x, int n, int peak) {
    float num = 0.0f;
    float den = 0.0f;
    AddPeakSqWeight(num, den, x, peak);
    if (peak > 0) {
        AddPeakSqWeight(num, den, x, peak - 1);
    }
    if (peak + 1 < n) {
        AddPeakSqWeight(num, den, x, peak + 1);
    }
    if (peak - 2 >= 0 && x[peak - 2] < x[peak - 1]) {
        AddPeakSqWeight(num, den, x, peak - 2);
    }
    if (peak + 2 < n && x[peak + 2] < x[peak + 1]) {
        AddPeakSqWeight(num, den, x, peak + 2);
    }
    return num / den;
}

// Reconstructed from eboot.elf at 0xE4710.
float FFTtoPeriod2(float* x, int n, int peak) {
    float center = x[peak];
    float left = peak > 0 ? x[peak - 1] : 0.0f;
    float right = peak + 1 < n ? x[peak + 1] : 0.0f;
    float mean = (right + left) * 0.5f;
    int neighbour = left > mean ? peak - 1 : peak + 1;
    float neighbourWeight = (left > mean ? left : right) - mean;
    float peakWeight = center - mean;
    return (neighbourWeight * neighbour + peakWeight * peak) / (neighbourWeight + peakWeight);
}

// Reconstructed from eboot.elf at 0xE4780.
void MakeWindowFunc(float* window, int n, int type) {
    switch (type) {
    case 0:
        for (int i = 0; i < n; ++i) {
            window[i] = 1.0f;
        }
        break;
    case 1: {
        int half = n / 2;
        for (int i = 0; i < half; ++i) {
            window[i] = 2.0f * i / n;
        }
        for (int i = half; i < n; ++i) {
            window[i] = 2.0f - 2.0f * i / n;
        }
        break;
    }
    case 2:
        for (int i = 0; i < n; ++i) {
            window[i] = 0.54f - 0.46f * cosf(i * 6.28318530717959f / n);
        }
        break;
    case 3: {
        float scale = 1.0f / n;
        for (int i = 0; i < n; ++i) {
            window[i] = 0.42f - 0.5f * cosf(i * 6.28318530717959f * scale)
                + 0.08f * cosf(i * 12.5663706143592f * scale);
        }
        break;
    }
    default:
        break;
    }
}
