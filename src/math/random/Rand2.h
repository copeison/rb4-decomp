#pragma once

// Minimal-standard Park-Miller generator. Only the seed is accessed by the
// recovered callers.
class Rand2 {
public:
    explicit Rand2(int seed) : mSeed(seed) {}

    int Int();

private:
    int mSeed;
};

static_assert(sizeof(Rand2) == 4);
