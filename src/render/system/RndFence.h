#pragma once

// GPU fence created by the factory's slot 2. The reference map predates
// fences; the base has only a virtual destructor. Name not in the reference
// map.
class RndFence {
public:
    virtual ~RndFence() {}
};
