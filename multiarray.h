#ifndef MULTIARRAY_H
#define MULTIARRAY_H

#include <cassert>
#include <iterator>
#include <memory>
#include <array>

template<typename T, unsigned int ndim>
class MultiArray
{
private:
    typedef unsigned long long int idx_t;
    typedef unsigned int smallidx_t;

    std::unique_ptr<idx_t[]> strides;
    idx_t size;
    std::unique_ptr<T[]> data;

    inline idx_t index(smallidx_t, smallidx_t i) const {
        return i;
    }

    template<typename ... Types>
    inline idx_t index(smallidx_t stridesidx, smallidx_t i, Types... rest) const {
        return i*strides[stridesidx]+index(stridesidx+1,rest...);
    }

    inline idx_t fill_strides(smallidx_t stridesidx,smallidx_t i) const {
        return strides[stridesidx]=i;
    }

    template<typename ... Types>
    inline idx_t fill_strides(smallidx_t stridesidx, smallidx_t i, Types... rest) const {
        return strides[stridesidx]=fill_strides(stridesidx+1,rest...)*i;
    }

    inline const T& operator[](idx_t idx) const {
        assert(idx<size);
        return data[idx];
    }

    inline T& operator[](idx_t idx) {
        assert(idx<size);
        return data[idx];
    }

    inline bool valid() const {
        return (strides||ndim==1) && data && size;
    }

public:
    template<typename ... Types>
    MultiArray(smallidx_t nfirst, Types... counts) :
        strides(new idx_t[ndim-1]),
        size(nfirst*fill_strides(0,counts...)),
        data(new T[size])
    {
        static_assert(ndim==sizeof...(counts)+1,"Invalid number of arguments in MultiArray constructor");
    }

    MultiArray(smallidx_t nfirst) :
        strides(nullptr),
        size(nfirst),
        data(new T[size])
    {
        static_assert(ndim==1,"Invalid number of arguments in MultiArray constructor");
    }

    MultiArray(const MultiArray &other) :
        strides(new idx_t[ndim-1]),
        size(other.size),
        data(new T[size])
    {
        assert(other.valid());
        std::copy(&other.strides[0],&other.strides[ndim-1],&strides[0]);
        std::copy(&other.data[0],&other.data[size],&data[0]);
    }

    MultiArray(MultiArray &&other) :
        strides(other.strides.release()),
        size(other.size),
        data(other.data.release())
    {
        other.size=0;
    }

    template<typename ... Types>
    inline const T& operator()(Types... indexes) const {
        static_assert(sizeof...(indexes)==ndim,"Invalid number of arguments in MultiArray::operator() const");
        assert(valid());
        return (*this)[index(0,indexes...)];
    }

    template<typename ... Types>
    inline T& operator()(Types... indexes) {
        static_assert(sizeof...(indexes)==ndim,"Invalid number of arguments in MultiArray::operator()");
        assert(valid());
        return (*this)[index(0,indexes...)];
    }

    class iterator : public std::iterator<std::forward_iterator_tag, T>
    {
    protected:
        friend class MultiArray;
        MultiArray* const arr;
        idx_t idx;
        iterator(MultiArray* arr, idx_t idx) : arr(arr), idx(idx) {}
    public:
        iterator& operator++() {++idx; return *this;}
        iterator operator++(int) {iterator tmp(*this); operator++(); return tmp;}
        bool operator==(const iterator& rhs) const {return !(*this!=rhs);}
        bool operator!=(const iterator& rhs) const {return arr!=rhs.arr || idx!=rhs.idx;}
        T& operator*() {return (*arr)[idx];}
        T* operator->() {return &(**this);}

        const MultiArray* parent() const { return arr; }
        const std::array<smallidx_t,ndim> index() const {
            assert(arr->valid());
            std::array<smallidx_t,ndim> i;
            auto t=idx;
            for(smallidx_t j=0; j<ndim-1; ++j) {
                i[j]=t/arr->strides[j];
                t=t%arr->strides[j];
            }
            i[ndim-1]=t;
            return i;
        }
    };

    class const_iterator : public std::iterator<std::forward_iterator_tag, T>
    {
    protected:
        friend class MultiArray;
        const MultiArray* arr;
        idx_t idx;
        const_iterator(const MultiArray* arr, idx_t idx) : arr(arr), idx(idx) {}
    public:
        const_iterator& operator++() {++idx; return *this;}
        const_iterator operator++(int) {const_iterator tmp(*this); operator++(); return tmp;}
        bool operator==(const const_iterator& rhs) const {return !(*this!=rhs);}
        bool operator!=(const const_iterator& rhs) const {return arr!=rhs.arr || idx!=rhs.idx;}
        const T& operator*() const {return (*arr)[idx];}
        const T* operator->() const {return &(**this);}

        const MultiArray* parent() const { return arr; }
        const std::array<smallidx_t,ndim> index() const {
            assert(arr->valid());
            std::array<smallidx_t,ndim> i;
            auto t=idx;
            for(smallidx_t j=0; j<ndim-1; ++j) {
                i[j]=t/arr->strides[j];
                t=t%arr->strides[j];
            }
            i[ndim-1]=t;
            return i;
        }
    };

    iterator begin() {
        return iterator(this,0);
    }

    iterator end() {
        return iterator(this,size);
    }

    template<typename ... Types>
    iterator make_iterator(Types... indices) {
        return iterator(this, index(0,indices...));
    }

    const_iterator const_begin() const {
        return const_iterator(this,0);
    }

    const_iterator const_end() const {
        return const_iterator(this,size);
    }

    template<typename ... Types>
    const_iterator make_const_iterator(Types... indices) const {
        return const_iterator(this, index(0,indices...));
    }
};

template<typename T, typename ... Types>
auto make_array(Types... counts) -> MultiArray<T,sizeof...(Types)> {
    return MultiArray<T,sizeof...(Types)>(counts...);
}
#endif // MULTIARRAY_H
