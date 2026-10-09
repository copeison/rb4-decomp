#pragma once

#include <cstddef>

#include "utl/containers/Vector.h"
#include "utl/text/Str.h"
#include "utl/text/Symbol.h"

// Performance budget categories, read from the system configuration's
// "budget_categories" array. The first category is always "~misc"; each
// configured one gives its name, a "gpu_budget" and the "filepaths" that
// belong to it. RndGpuStatsMgr creates one GPU statistic per category.
//
// Not in the reference map: the object, the type, the global and the
// functions are newer than the map's build. In this build the code follows
// the utl GlitchFinder code.
struct BudgetCategory {
    BudgetCategory() : mGpuBudget(0.0F) {}
    // Copies the paths into storage of exactly their count.
    BudgetCategory(const BudgetCategory& other);

    Symbol mName;
    float mGpuBudget;
    eastl::vector<String> mFilePaths;
};

static_assert(offsetof(BudgetCategory, mGpuBudget) == 8);
static_assert(offsetof(BudgetCategory, mFilePaths) == 16);
static_assert(sizeof(BudgetCategory) == 48);

// The categories, at 0x19F2700. Its destructor is at 0x25EC00.
extern eastl::vector<BudgetCategory> gBudgetCategories;

// Fills gBudgetCategories; SystemInit calls it through the timer setup at
// 0x249610, after Hmx::Timer::Init.
void InitBudgetCategories();  // 0x25ECB0
unsigned long NumBudgetCategories();       // 0x25F2D0
Symbol BudgetCategoryName(int index);      // 0x25F300
float BudgetCategoryGpuBudget(int index);  // 0x25F320
