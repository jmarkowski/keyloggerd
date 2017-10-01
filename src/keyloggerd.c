#include <unistd.h>

void keyloggerd(void)
{
#if DEBUG
    /* Dummy process so that I can test the daemon */
    sleep(60);
#endif
}
