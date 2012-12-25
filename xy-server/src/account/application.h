#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include "account_infoI.h"

class Application
{
public:
    struct AppCfg
    {
        struct IBusCfg
        {
            int selfID;
            int dbsrvID;
        } ibusCfg;
    };

    ~Application();
    int Initialize(int _argc, char* _argv[]);
    void MainLoop();
    void ExitLoop();

    inline AccountInfoI& GetAccountInfoInterface()
    {
        return accountInfoI_;
    }

    inline MapEmailAccount2* GetTransanctionList()
    {
        return tList_;
    }

    inline const AppCfg& GetCfg() const
    {
        return appCfg_;
    }
private:
    void ParserCmdLine(int _argc,  char* _argv[]);
    int  Tick(long _tick);
    int  MsgHandle();
    int  CreateGlobalObject();
    void InitLog();
    int  ReadCfg(const char* _cfgName);
    int  ReadIBusCfg(const char* _cfgName);

    AccountInfoI accountInfoI_;
    static const unsigned int MaxMsgLength = 1024 * 1024;
    char msgBuff_[MaxMsgLength];
    AppCfg appCfg_;
    int  tickCount_;
    bool resume_;
    bool loop_cont_;
    MapEmailAccount2 * tList_;
};

#endif
