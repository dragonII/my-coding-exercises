#include <unistd.h>

#include <iostream>

#include <log4cpp/PropertyConfigurator.hh>
#include <log4cpp/BasicConfigurator.hh>
#include <google/protobuf/message_lite.h>
#include <google/protobuf/stubs/common.h>

#include "application.h"

void DealIBusMsg(const char* _msg, int _msgLength, int _msgSrcID);

log4cpp::Category* gCategory = 0;

Application::~Application()
{
    google::protobuf::ShutdownProtobufLibrary();
}


int Application::Initialize(int _argc, char* _argv[])
{
    const char* cfgFileName = "../config/svr.cfg";

    GOOGLE_PROTOBUF_VERIFY_VERSION;
    ParserCmdLine(_argc, argv);
    InitLog();

    if(ReadCfg(cfgFileName) != 0)
    {
        return -2;
    }

    if(CreateGlobalObject() != 0)
    {
        return -3;
    }

    loop_cont_ = true;
    return 0;
}

void Application::MainLoop()
{
    TimeUtils::tick_mem_fun1_t<int, Application, long> tickFunc
            = TimeUtils::tick_mem_fun(&Application::Tick, this);
    while(loop_cont_)
    {
        int msgCount = MsgHandle();

        tickCount_ = 0;
        TimeUtils::Tick(tickFunc);

        if(msgCount == 0 && tickCount_ == 0)
        {
            usleep(10);
            continue;
        }
    }
}

int Application::Tick(long _tick)
{
    return 0;
}

int Application::MsgHandle()
{
    int count = 0;
    const int MaxMsgHandleCount = 30;
    for(; count < MaxMsgHandleCount; ++count)
    {
        int srcID = RECV_ID_ANY;
        int ret = ibus::RecvMsg(msgBuff_, MaxMsgLength, srcID);
        if(ret < 0)
        {
            LOG_FATAL("Failed to recv msg!");
            break;
        }
        else if(ret == 0)
        {
            break;
        }
        DealIBusMsg(msgBuff_, ret, srcID);
    }
    return count;
}

int Application::CreateGlobalObject()
{
    const void* PreDefinePtr = (const void*)0x10000000;
    CShmWrapper shm;
    if(shm.CreateShm(appCfg_.cacheCfg.key, appCfg_.cacheCfg.size, PreDefinePtr, "Account System Cache && Config Info v1.0", resume_) != 0)
    {
        return -2;
    }
    ConstructObj(&TimeUtils::gTickData, shm.Malloc(sizeof(TimeUtils::TickData)), resume_);
    LOG_INFO("Shm malloc TimeUtils::TickData, size: %1%, offset: %2%",
                sizeof(TimeUtils::TickData), shm.GetShmOffset());

    ConstructObj(&tList_, shm.Malloc(sizeof(MapEmailAccount2)), resume_);
    LOG_INFO("Shm malloc MapEmailAccount2, size: %1%, offset: %2%",
                sizeof(MapEmailAccount2), shm.GetShmOffset());

    return 0;
}

void Application:InitLog()
{
    const char* cfgFileName = "../config/account_log.properties";
    try
    {
        log4cpp::PropertyConfigurator::configure(cfgFileName);
        gCategory = &log4cpp::Category::getRoot();
        return;
    } catch(const log4cpp::ConfigureFailure& _err)
    {
        std::cerr << "log4cpp configure failure: " << _err.what() << std::endl;
    } catch(std::exception& _err)
    {
        std::cerr << "exception: " << _err.what() << std::endl;
    }

    // If error occured
    log4cpp::BasicConfigurator::configure();
    gCategory = &log4cpp::Category::getRoot();
}

int Application::ReadCfg(const char* _cfgName)
{
    config_t    configData;
    config_t*   conf = &configData;
    CConfigGuard guard(conf);

    if(config_read_file(conf, _cfgName) == CONFIG_FALSE)
    {
        LOG_WARN("%s:%d failed to parse. %s",
            _cfgName, config_error_line(conf), config_error_text(conf));
        return -1;
    }

    config_setting_t* subCfg = config_lookup(conf, "account.cache");
    config_setting_lookup_int(subCfg, "key", &appCfg_.cacheCfg.key);
    const char* size;
    config_setting_lookup_string(subCfg, "size", &size);
    appCfg_.cacheCfg.size = AscStrToSize(size);

    return 0;
}

void Application::ExitLoop()
{
    loop_cont_ = false;
}

void Application::ParserCmdLine(int _argc, char* _argv[])
{
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    std::string pidfile;
    desc.add_options()
        ("help,h",      "produce help message")
        ("daemon,d",    "Run in daemon")
        ("resume,r",    "Run in resume")
        ("pidfile,p",   po::value<std::string>(&pidfile),
                        "Write pidfile and prevent the same process")
        ("version,V"    "Print version information")
        ;

    po::variables_map vm;
    po::store(po::command_line_parser(_argc, _argv).options(desc).allow_unregistered().run(), vm);
    po::notify(vm);

    if(vm.count("help"))
    {
        std::cout << desc << std::endl;
        exit(0);
    }

    if(vm.count("version"))
    {
        std::cout << _argv[0] << ":" << std::endl
                  << "\t\tBuild on" << __DATE__ << std::endl;
        exit(0);
    }

    if(vm.count("daemon"))
    {
        InitDaemon();
    }

    if(vm.count("pidfile"))
    {
        if(RunOneInstance(pidfile.c_str()))
        {
            fprintf(stderr, "Failed to lock pid file %s\n", pidfile.c_str());
            exit(1);
        }
    }

    if(vm.count("resume"))
    {
        resume_ = true;
    } else
    {
        resume_ = false;
    }
}


int Application::ReadIBusCfg(const char* _cfgName)
{
    int ret = ibus::AttachIBUS(_cfgName);
    if(ret == 0)
    {
        appCfg_.ibusCfg.selfID = ibus::GetProcID("account");
        appCfg_.ibusCfg.dbsrvID = ibus::GetProcID("dbsvr");
        ibus::AddDefaultProcID(appCfg_.ibusCfg.selfID);
    }

    return ret;
}
