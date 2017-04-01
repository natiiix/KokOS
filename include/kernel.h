#pragma once

#if defined(__cplusplus)
extern "C"
{
#endif

void kernel_panic(const char* const str);

void debug_memusage(void);

#if defined(__cplusplus)
}
#endif
