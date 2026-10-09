#include "utl/profiling/BudgetCategories.h"

#include "os/system/System.h"
#include "utl/data/DataArray.h"

eastl::vector<BudgetCategory> gBudgetCategories;

BudgetCategory::BudgetCategory(const BudgetCategory& other)
    : mName(other.mName), mGpuBudget(other.mGpuBudget) {
    mFilePaths.reserve(other.mFilePaths.size());
    for (const auto& path : other.mFilePaths) {
        mFilePaths.push_back(path);
    }
}

// Reconstructed from eboot.elf at 0x25ECB0. Each configured entry is an
// array: the category name, then keyed settings.
void InitBudgetCategories() {
    static Symbol sFilePaths;
    if (sFilePaths == Symbol()) {
        sFilePaths = Symbol("filepaths");
    }

    const DataArray* config = SystemConfig(Symbol("budget_categories"));
    const unsigned long count =
        config != nullptr ? static_cast<unsigned int>(config->Size()) : 1;
    if (gBudgetCategories.capacity() < count) {
        gBudgetCategories.reserve(count);
    }

    {
        BudgetCategory misc;
        misc.mName = Symbol("~misc");
        gBudgetCategories.push_back(misc);
    }

    if (config == nullptr) {
        return;
    }
    for (int i = 1; i < config->Size(); ++i) {
        BudgetCategory category;
        const DataArray* entry = config->Node(static_cast<unsigned long>(i)).mValue.array;
        category.mName = entry->Sym(0);

        static Symbol sGpuBudget;
        if (sGpuBudget == Symbol()) {
            sGpuBudget = Symbol("gpu_budget");
        }
        float gpuBudget = 0.0F;
        entry->FindData(sGpuBudget, gpuBudget, false);
        category.mGpuBudget = gpuBudget;

        const DataArray* paths = entry->FindArray(sFilePaths, false);
        if (paths != nullptr) {
            category.mFilePaths.reserve(static_cast<unsigned long>(paths->Size() - 1));
            for (int j = 1; j < paths->Size(); ++j) {
                category.mFilePaths.push_back(String(paths->Str(static_cast<unsigned long>(j))));
            }
        }
        gBudgetCategories.push_back(category);
    }
}

// Reconstructed from eboot.elf at 0x25F2D0.
unsigned long NumBudgetCategories() {
    return gBudgetCategories.size();
}

// Reconstructed from eboot.elf at 0x25F300.
Symbol BudgetCategoryName(int index) {
    return gBudgetCategories[static_cast<unsigned long>(index)].mName;
}

// Reconstructed from eboot.elf at 0x25F320.
float BudgetCategoryGpuBudget(int index) {
    return gBudgetCategories[static_cast<unsigned long>(index)].mGpuBudget;
}
