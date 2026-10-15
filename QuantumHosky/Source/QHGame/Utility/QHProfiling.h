#pragma once

#include "HAL/PlatformMisc.h"

//------------------------------------------------------------------------------

#define QH_PROFILING_ENABLED !UE_BUILD_SHIPPING

#if QH_PROFILING_ENABLED

#define QH_PROFILE_SCOPE(InNameStatic) SCOPED_NAMED_EVENT(InNameStatic, FColor::White)
#define QH_PROFILE_SCOPE_STR(InString) SCOPED_NAMED_EVENT_TCHAR(InString, FColor::White)
#define QH_PROFILE_SCOPE_PRINTF(InFormat, ...) SCOPED_NAMED_EVENT_F(InFormat, FColor::White, __VA_ARGS__)
#define QH_PROFILE_FUNCTION() SCOPED_NAMED_EVENT_TCHAR(ANSI_TO_PROFILING(__FUNCTION__), FColor::White)

#else //~ QH_PROFILING_ENABLED

#define QH_PROFILE_SCOPE(...)
#define QH_PROFILE_SCOPE_STR(...)
#define QH_PROFILE_SCOPE_PRINTF(...)
#define QH_PROFILE_FUNCTION(...)

#endif //~ else // QH_PROFILING_ENABLED
