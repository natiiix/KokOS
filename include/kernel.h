#pragma once

#if defined(__cplusplus)
extern "C"
{
#endif

void kernel_panic(const char* const str);

#ifdef DEBUG
void debug_memusage(void);
#endif

#if defined(__cplusplus)
}
#endif
