
//////////////////////////////////////////////////////////////////////////
//#define _WIN32_WINNT 0x0601
#include "test_head.h"

//////////////////////////////////////////////////////////////////////////

void dummy(void);

int main(void)
{
    cout << __cplusplus << endl;
    dummy();
    testAll();
    return 0;
}
