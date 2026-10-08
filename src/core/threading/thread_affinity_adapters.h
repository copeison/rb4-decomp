#pragma once

namespace rb4 {

struct ThreadAffinityGroup;

const ThreadAffinityGroup* thread_affinity_find_group(const char* name);

}  // namespace rb4
