#ifndef __ACCOUNT_INFO_I__H
#define __ACCOUNT_INFO_I__H

#ifndef MAX_ACCOUNT_COUNT
#define MAX_ACCOUNT_COUNT 1000
#endif

#ifndef MD5_DIGEST_LENGTH
#define MD5_DIGEST_LENGTH   16
#endif

struct AccountInfo
{
    static const int MAX_EMAIL_CMP_LENGTH = 32;
    static const int MAX_EMAIL_LENGTH     = 64;
    unsigned     int uin;
    unsigned     int lastIP;
    unsigned     int lastSvrID;
    time_spec_type   lastTime;
    size_t           hashOfEmail;
    bool             isOnLine;
    unsigned    char passwdMD5[MD5_DIGEST_LENGTH];
    char             email[MAX_EMAIL_LENGTH];

    AccountInfo()
    {
        isOnLine = false;
    }

    bool GetAccountOnlineStatus()
    {
        return isOnLine;
    }

    void setAccountOnline()
    {
        isOnLine = true;
    }

    void SetAccountOffline()
    {
        isOnLine = false;
    }
};

struct AccountInfo2
{
    AccountInfo     accountInfo;
    unsigned int    lastReqIP;
    unsigned char   signature[MD5_DIGEST_LENGTH];
};

struct ExtractAccountInfo
{
    inline const char* operator() (const AccountInfo& _info) const
    {
        return _info.email;
    }
};

struct ExtractAccountInfo2
{
    inline const char* operator() (const AccountInfo2 _info) const
    {
        return _info.accountInfo.email;
    }
};

struct EqualAccountInfo
{
    inline bool operator() (const char* _info1, const char* _info2) const
    {
        return strcmp(_info1, _info2);
    }
};

typedef SimpleHashtableT<AccountInfo, const char*, detail::hash<const char*>,
            EqualAccountInfo, ExtractAccountInfo, 1000, 1000> MapEmailAccount;

typedef SimpleHashtableT<AccountInfo2, const char*, detail::hash<const char*>,
            EqualAccountInfo, ExtractAccountInfo2, 1000, 1000> MapEmailAccount2;

typedef SimpleMap<unsigned int, index_type, MAX_ACCOUNT_COUNT, MAX_ACCOUNT_COUNT> MapUinAccount;

class AccountInfoI
{
public:
    int AddAccount(const AccountInfo& _account);
    int DeleteAccount(unsigned int _uin);
    AccountInfo* FindByUin(unsigned int _uin);
    AccountInfo* FindByEmail(const char* _email);
    int GetSID(const AccountInfo* _info) const;
};



#endif
