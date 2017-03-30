#include <c/stdlib.h>
#include <c/stdio.h>

extern "C"
void shell_init(void)
{
    setbg(VGA_COLOR_RED);
    setfg(VGA_COLOR_LIGHT_GREEN);
    clear();
    printat("Hello world!", 20, 10);

    setactive(20, 15);
    char* strin = scan();
    setbg(VGA_COLOR_BLUE);
    setfg(VGA_COLOR_WHITE);
    print(strin);
    delete strin;
    setcursor(40, 5);

    while (true);
}
