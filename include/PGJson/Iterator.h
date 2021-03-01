//
// Created by 42025 on 2021/3/1.
//
#ifndef PGTEST_ITERATOR_H
#define PGTEST_ITERATOR_H

#include <iterator>
#include <type_traits>
#include <PGJson/fwd.h>
#include <iostream>

PGJSON_NAMESPACE_START



template<typename DataType>
class PoPSeqIterator {
    using NonConstDataType = typename std::remove_const<DataType>::type;
    using ConstDataType = typename std::add_const<DataType>::type;
//    static_assert(std::is_same<NonConstDataType, DataType>::value, "");
    friend class PoPSeqIterator<NonConstDataType>;
    friend class PoPSeqIterator<ConstDataType>;

    using PoPType = NonConstDataType **;
    using SelfType = PoPSeqIterator<DataType>;

public:
    typedef DataType value_type;
    typedef value_type * pointer;
    typedef value_type & reference;
    typedef std::ptrdiff_t difference_type;
    typedef std::random_access_iterator_tag iterator_category;

    PoPSeqIterator() = default;

    explicit PoPSeqIterator(NonConstDataType ** pPtr) : m_ptr(pPtr) {
    }

    PoPSeqIterator(const PoPSeqIterator<NonConstDataType> & other) : m_ptr(other.m_ptr) {
    }

    SelfType & operator= (const PoPSeqIterator<NonConstDataType> & other) {
        if (this == &other) return *this;

        m_ptr = other.m_ptr;
        return *this;
    }

    SelfType & operator++ () { ++m_ptr; return *this; }
    SelfType & operator-- () { --m_ptr; return *this; }
    SelfType operator++ (int) { SelfType old(*this); ++m_ptr; return old; }
    SelfType operator-- (int) { SelfType old(*this); --m_ptr; return old; }

    SelfType operator+ (difference_type n) const { return SelfType(m_ptr + n); }
    SelfType operator- (difference_type n) const { return SelfType(m_ptr - n); }

    difference_type operator- (const PoPSeqIterator<NonConstDataType> & other) const { return m_ptr - other.m_ptr; }
    difference_type operator- (const PoPSeqIterator<ConstDataType> & other) const { return m_ptr - other.m_ptr; }

    SelfType & operator+= (difference_type n) { m_ptr += n; return *this; }
    SelfType & operator-= (difference_type n) { m_ptr -= n; return *this; }

    bool operator== (const PoPSeqIterator<NonConstDataType> & other) const { return m_ptr == other.m_ptr; }
    bool operator!= (const PoPSeqIterator<NonConstDataType> & other) const { return m_ptr != other.m_ptr; }
    bool operator<= (const PoPSeqIterator<NonConstDataType> & other) const { return m_ptr <= other.m_ptr; }
    bool operator>= (const PoPSeqIterator<NonConstDataType> & other) const { return m_ptr >= other.m_ptr; }
    bool operator<  (const PoPSeqIterator<NonConstDataType> & other) const { return m_ptr < other.m_ptr; }
    bool operator>  (const PoPSeqIterator<NonConstDataType> & other) const { return m_ptr > other.m_ptr; }

    bool operator== (const PoPSeqIterator<ConstDataType> & other) const { return m_ptr == other.m_ptr; }
    bool operator!= (const PoPSeqIterator<ConstDataType> & other) const { return m_ptr != other.m_ptr; }
    bool operator<= (const PoPSeqIterator<ConstDataType> & other) const { return m_ptr <= other.m_ptr; }
    bool operator>= (const PoPSeqIterator<ConstDataType> & other) const { return m_ptr >= other.m_ptr; }
    bool operator<  (const PoPSeqIterator<ConstDataType> & other) const { return m_ptr < other.m_ptr; }
    bool operator>  (const PoPSeqIterator<ConstDataType> & other) const { return m_ptr > other.m_ptr; }

    reference operator*() const { return **m_ptr; }
    pointer   operator->() const { return *m_ptr; }
    reference operator[] (difference_type n) const { return *m_ptr[n]; }

protected:
    PoPType m_ptr = nullptr;
};

PGJSON_NAMESPACE_END
#endif //PGTEST_ITERATOR_H
