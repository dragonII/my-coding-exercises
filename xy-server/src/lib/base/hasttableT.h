#ifndef __HASHTABLE_LIST_H__
#define __HASHTABLE_LIST_H__

#include "sys_types.h"

#include <iostream>
#include <string.h>

/*
 * HastTableListT uses hash value to contain
 * data, the key of hash is unique. It's user's
 * responsibility to maintain the uniqueness of
 * the key value, or something goes wrong.
 * 
 * find values according to 
 *  1. user name
 *  2. account info
 */

template <typename PODType, typename Key, typename Hash, typename Eq,
            typename ExtractKey, size_t BucketCount, size_t MaxNodeNum>
class SimpleHashTableT : private ArrayListT <PODType, BucketCount, MaxNodeNum>
{
public:
    typedef ArrayListT<PODType, BucketCount, MaxNodeNum> super;
    typedef typename super::value_type value_type;
    typedef typename super::reference reference;
    typedef typename super::const_reference const_reference;
    typedef typename super::pointer pointer;
    typedef typename super::const_pointer const_pointer;
    typedef typename super::iterator iterator;
    typedef typename super::const_iterator const_iterator;

    SimpleHashTableT() {}

    inline iterator Find(const Key& _key)
    {
        std::pair<iterator, iterator> pair = GetLink(HashKey(_key));
        return Find(pair.first, pair.second, _key);
    }

    inline const_iterator Find(const Key& _key) const
    {
        std::pair<const_iterator, const_iterator> pair = GetLink(HashKey(_key));
        return Find(pair.first, pair.second, _key);
    }

    inline iterator End()
    {
        return super::End();
    }

    inline const_iterator End() const
    {
        return super::End();
    }

    inline index_type Insert(const value_type& _value)
    {
        index_type hash = HashKey(extractKey(_value));
        return super::push_front(hash, _value);
    }

    inline reference operator[](index_type _ind)
    {
        return super::operator[](_ind);
    }

    inline const_reference operator[](index_type _ind) const
    {
        return super::operator[](_ind);
    }

    inline iterator Remove(iterator _itr)
    {
        return super::Remove(_itr);
    }

    inline void Remove(const Key& _key)
    {
        iterator itr = Find(_key);
        if(itr != End())
            Remove(itr);
    }

    inline index_type Index(const_pointer _p) const
    {
        return super::Index(_p);
    }

private:
    inline const_iterator Find(const_iterator _first, const_iterator _last, const Key& _key) const
    {
        for(; _first != _last; ++_first)
            if(eq(extractKey(*_first), _key))
                break;
        return _first;
    }

    inline iterator Find(iterator _first, iterator _last, const Key& _key)
    {
        for(; _first != _last; ++_first)
            if(eq(extractKey(*_first), _key))
                break;
        return _first;
    }

    inline index_type HashKey(const Key& _key) const
    {
        return index_type(hash(_key) % BucketCount);
    }

    Hash        hash;
    Eq          eq;
    ExtractKey  extractKey;
};

template <typename KeyType, typename ValType, size_t BucketCount, size_t MaxNodeNum>
class SimpleMap : public SimpleHashTableT<std::pair<KeyType, ValType>, KeyType,
                        detail::hash<KeyType>, std::equal_to<KeyType>, std::_Selectlst<std::pair<KeyType, ValType> >,
                        BucketCount, MaxNodeNum>
{
};

template <typename ValType, size_t Length>
struct StrMapData
{
    StrMapData() {}
    StrMapData(const char* _key, const ValType& _value) : second(_value)
    {
        strncpy(first, _key, Length);
        first[Length - 1] = 0;
    }

    char    first[Length];
    ValType second;
};

template <typename DataType>
struct ExtractKey
{
    inline const char* operator() (const DataType& _data) const
    {
        return _data.first;
    }
};

struct EqStr
{
    inline bool operator()(const char* _left, const char* _right)
    {
        return (strcmp(_left, _right) == 0);
    }
};

template <size_t KeyLength, typename ValType, size_t BucketCount, size_t MaxNodeNum>
struct SimpleStrMap : public SimpleHashTableT<StrMapData<ValType, KeyLength>, const char*,
                        detail::hast<const char*>, EqStr, ExtractKey<StrMapData<ValType, KeyLength>>,
                        BucketCount, MaxNodeNum>
{};                        


#endif
