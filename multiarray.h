#ifndef MULTIARRAY_H
#define MULTIARRAY_H

#include <cassert>

template<typename T>
class MultiArray
{
private:
    typedef unsigned long long int idx_t;
    typedef unsigned int smallidx_t;

    smallidx_t ndim;
    idx_t* strides;
    idx_t size;
    T* data;

    inline idx_t index(smallidx_t stridesidx) const {
        assert(stridesidx==ndim);
        return 0;
    }

    template<typename ... Types>
    inline idx_t index(smallidx_t stridesidx, smallidx_t i, Types... rest) const {
        assert(stridesidx<ndim);
        return i*strides[stridesidx]+index(stridesidx+1,rest...);
    }

    inline idx_t fill_strides(smallidx_t stridesidx) const {
        assert(stridesidx==ndim-1);
        strides[stridesidx]=1;
        return 1;
    }

    template<typename ... Types>
    inline idx_t fill_strides(smallidx_t stridesidx, smallidx_t i, Types... rest) const {
        assert(stridesidx<ndim);
        return strides[stridesidx]=fill_strides(stridesidx+1,rest...)*i;
    }
public:
    template<typename ... Types>
    MultiArray(smallidx_t nfirst, Types... counts) {
        ndim=sizeof...(counts)+1;
        strides=new idx_t[ndim];
        fill_strides(0,counts...);
        size=nfirst*strides[0];
        data=new T[size];
    }

    ~MultiArray() {
        delete[] strides;
        delete[] data;
    }

    template<typename ... Types>
    inline T& operator()(Types... indexes) const {
        idx_t idx=index(0,indexes...);
        assert(idx<size);
        return data[idx];
    }
};
#endif // MULTIARRAY_H
