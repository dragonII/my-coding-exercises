#ifndef __ARRAYLIST_TEMPLATE_H__
#define __ARRAYLIST_TEMPLATE_H__

/*
 * Combined List and Array together, to achieve
 *  1. data can be stored in shared-memory
 *  2. can access List by index
 */

#include "sys_types.h"

#include <iterator>

struct SLIteratorBase
{
    typedef std::forward_iterator_tag iterator_category;
    typedef ptrdiff_t   difference_type;

    SLIteratorBase() : link(0), prev(0) {}

    SLIteratorBase(const index_type* _link, index_type _prev)
        : link(_link), prev(_prev) {}

    SLIteratorBase(const SLIteratorBase& _x)
        : link(_x.link), prev(_x.prev) {}

    inline bool operator==(const SLIteratorBase& _x) const
    {
        return link[prev] == _x.link[_x.prev];
    }

    inline bool operator!=(const SLIteratorBase& _x) const
    {
        return !(*this == _x);
    }

    const index_type* link;   
    index_type        prev;
};


template <typename PODType, typename Reference, typename Pointer>
struct SLIterator : public SLIteratorBase
{
    typedef PODType     value_type;
    typedef Pointer     pointer;
    typedef Reference   reference;

    typedef SLIterator<PODType, PODType&, PODType*>             iterator;
    typedef SLIterator<PODType, const PODType&, const PODType*> const_iterator;
    typedef SLIterator<PODType, Reference, Pointer>             self_type;

    SLIterator() : data(0) {}
    SLIterator(const iterator& _x) : SLIteratorBase(_x), data(_x.data) {}
    SLIterator(const const_iterator& _x) : SLIteratorBase(_x), data(_x.data) {}
    SLIterator(const index_type* _link, index_type _prev, pointer _data)
                    : SLIteratorBase(_link, _prev, _data) {}

    reference operator*() const
    {
        return data[link[prev]];
    }

    pointer operator->() const
    {
        return &(operator*());
    }

    inline self_type& operator++()
    {
        prev = link[prev];
        return *this;
    }

    inline self_type operator++(int)
    {
        self_type tmp(*this);
        prev = link[prev];
        return tmp;
    }

    pointer data;
};

template <typename PODType, size_t ArrayCount, size_t MaxNodeNum>
class ArrayListT
{
public:
    typedef PODType             value_type;
    typedef value_type&         reference;
    typedef const value_type&   const_reference;
    typedef value_type*         pointer;
    typedef const value_type*   const_pointer;
    typedef SLIterator<value_type, reference, pointer>              iterator;
    typedef SLIterator<value_type, const_reference, const_pointer>  const_iterator;
    static  const size_t BUCKET_OFFSET = MaxNodeNum + 2;

    ArrayListT()
    {
        Init();
    }

    void Init()
    {
        size_t i = 0;
        for(; i < MaxNodeNum - 1; ++i)
        {
            link_[i] = i + 1;
        }
        link_[i++] = TAIL_OF_LIST;
        link_[i++] = 0;

        memset(link_ + i, TAIL_OF_LIST, sizeof(link_[i]) * (BUCKET_OFFSET + ArrayCount - i));
    }

    inline std::pair<const_iterator, const_iterator> GetLink(index_type _ind) const
    {
        assert(_ind >= 0 && _ind < int(ArrayCount));
        return std::make_pair(const_iterator(link_, _ind + BUCKET_OFFSET, data_), End());
    }

    inline std::pair<iterator, iterator> GetLink(index_type _ind)
    {
        assert(_ind >= 0 && _ind < int(ArrayCount));
        return std::make_pair(iterator(link_, _ind + BUCKET_OFFSET, data_), End());
    }

    inline index_type push_front(index_type _ind, const value_type& _value)
    {
        pointer value = mutable_push_front(_ind);
        if(value == 0)
            return -1;
        *value = _value;
        return value - data_;
    }

    inline pointer mutable_push_front(index_type _ind)
    {
        assert(_ind >= 0 && _ind < int(ArrayCount));
        if(IsTail(link_[MaxNodeNum]))
            return 0;

        index_type freeNode         = link_[MaxNodeNum];
        link_[MaxNodeNum]           = link_[freeNode];
        link_[freeNode]             = link_[_ind + BUCKET_OFFSET];
        link_[_ind + BUCKET_OFFSET] = freeNode;
        return data_ + freeNode;
    }

    inline reference operator[](index_type _ind)
    {
        return data_[_ind];
    }

    inline const_reference operator[](index_type _ind) const
    {
        return data_[_ind];
    }

    inline iterator Remove(iterator _itr)
    {
        assert(_itr != End());
        index_type prevNode = _itr.prev;
        index_type currNode = link_[prevNode];
        link_[prevNode]     = IsTail(currNode) ? TAIL_OF_LIST : link_[currNode];
        link_[currNode]     = link_[MaxNodeNum];
        link_[MaxNodeNum]   = currNode;
        return _itr;
    }

    inline iterator End()
    {
        return iterator(link_, BUCKET_OFFSET - 1, data_);
    }

    inline const_iterator End() const
    {
        return const_iterator(link_, BUCKET_OFFSET - 1; data_);
    }

    inline index_type Index(const_pointer _p) const
    {
        assert(_p >= data_ && _p < data_ + MaxNodeNum);
        return _p - data_;
    }

private:
    static const int TAIL_OF_LIST   = -1;
    inline bool IsTail(index_type _ind) const
    {
        return TAIL_OF_LIST == _ind;
    }

    index_type link_[BUCKET_OFFSET + ArrayCount];
    value_type data_[MaxNodeNum];
};
        



#endif
