#ifndef __CONFIG_GUARD_H__
#define __CONFIG_GUARD_H__

#include <libconfig.h>

class CConfigGuard
{
public:
    CConfigGuard(config_t* _conf) : conf_(_conf)
    {
        config_init(conf_);
    }

    ~CConfigGuard()
    {
        config_destroy(conf_);
    }
private:
    config_t* conf_;
};



#endif
