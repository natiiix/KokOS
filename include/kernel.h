#pragma once

#if defined(__cplusplus)
extern "C"
{
#endif

void kernel_panic(void);
void kernel_panic_default(void);

void debug_memusage(void);

#if defined(__cplusplus)
}
#endif
