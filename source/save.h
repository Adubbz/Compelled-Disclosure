#pragma once

#include <switch.h>

Result backupSystemSavedata(u64 titleId, u64 saveId);
Result restoreSystemSavedata(u64 titleId, u64 saveId);