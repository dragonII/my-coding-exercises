

Application theApp;
int main(int argc, char* argv[])
{
    if(theApp.Initialize(argc, argv) == 0)
    {
        theApp.MainLoop();
        if(gCategory != 0)
            log4cpp::Category::shutdown();

        return 0;
    }
    return -1;
}
