#include "lib/base/log_wrap.h"
#include "application.h"

Application theApp;

int main(int argc, char** argv)
{
    if(theApp.Initialize(argc, argv) == 0)
    {
        theApp.MainLoop();
        if(gCategory != 0)
        {
            log4cpp::Category::shutdown();  // log4cpp must be initialized
                                            // before calling this function,
                                            // or core dump
        }
        return 0;
    }
    return -1;
}
