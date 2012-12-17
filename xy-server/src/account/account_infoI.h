#ifndef __ACCOUNT_INFO_I_H__
#define __ACCOUNT_INFO_I_H__

#ifndef
#define MAX_ACCOUNT_COUNT   1000
#endif

#include "lib/base/sys_types.h"
#include <string.h>

/*
 * email    --> uin + account_info
 * uin      --> account info
 * 
 * array: uin + account_info
 * hash:  key = email, value = hash(email), array index
 * hash:  key = uin,   value = array index
 */

#ifndef MD5_DIGEST_LENGTH
#define MD5_DIGEST_LENGTH 16
#endif

/*
 * In account information, UIN and email cannot be changed.
 */

struct AccountInfo
{
    static const    int MAX_EMAIL_CMP_LENGTH = 32; // Only 32 charaters from beginning are used to identify a user
    static const    int MAX_EMAIL_LENGTH = 64;     // email address length, according to DB allocated space
    unsigned int    uin;            // identification for user, primary key
    unsigned int    lastIP;         // last login IP address
    unsigned int    lastSvrID;      // last server which provided services
    time_spec_type  lastTime;       // last login time
    size_t          hasOfEmail;     // email hash 
    bool            isOnLine;
    unsigned char   passwdMD5[MD5_DIGEST_LENGTH];   // user passwd in MD5
    char            email[MAX_EMAIL_LENGTH]

    AccountInfo()
    {
        isOnLine = false;
    }

    bool GetAccountOnlineStatus()
    {
        return isOnLine;
    }

    void SetAccountOnline()
    {
        isOnLine = true;
    }

    void SetAccountOffline()
    {
        isOnLine = false;
    }
};

struct AccountInfoDetail
{
    AccountInfo     accountInfo;
    unsigned int    lastReqIP;      // last client IP
    unsigned char   signature[MD5_DIGEST_LENGTH];
};

struct ExtractAccountInfo1
{
    inline const char* operator() (const AccountInfo& _info) const
    {
        return _info.email;
    }
};

struct ExtractAccountInfo2
{
    inline const char* operator() (const AccountInfoDetail& _info) const
    {
        return _info.account_info.email;
    }
};

struct EqualAccountInfo
{
    inline bool operator() (const char* _info1, const char* _info2) const
    {
        return (strcmp(_info1, _info2) == 0);
    }
};

typedef SimpleHashtableT<AccountInfo, const char*, detail::hash<const char*>,
            EqualAccountInfo, ExtractAccountInfo1, 1000, 1000> MapEmailAccount;

typedef SimpleHashtableT<AccountInfoDetail, const char*, detail::hash<const char*>
            EqualAccountInfo, ExtractAccountInfo2, 1000, 1000> MapEmailAccountDetail;

typedef SimpleMap<unsigned int, index_type, MAX_ACCOUNT_COUNT, MAX_ACCOUNT_COUNT> MapUinAccount;

class AccountInfoDetail
{
    public:
    int AddAccount(const AccountInfo& _account);
    int DelAccount(unsigned int _uin);
    AccountInfo* FindByUin(unsigned int _uin);
    AccountInfo* FindByEmail(const char* _email);

    /*
     * Get account info index in array
     */
    int GetSID(const AccountInfo* _info) const;
};

#endif /* __ACCOUNT_INFO_I_H__ */
