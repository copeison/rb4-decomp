#pragma once

namespace rb4 {

struct OrbisFence;

OrbisFence* orbis_create_fence();
void orbis_fence_construct(OrbisFence& fence);
void orbis_fence_destruct(OrbisFence& fence);
void orbis_fence_base_destruct(OrbisFence& fence);
void orbis_fence_delete(OrbisFence& fence);

}  // namespace rb4
