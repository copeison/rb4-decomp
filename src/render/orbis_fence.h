#pragma once

namespace rb4 {

struct OrbisFence;

OrbisFence* orbis_create_fence();
void orbis_fence_construct(OrbisFence& fence);

}  // namespace rb4
