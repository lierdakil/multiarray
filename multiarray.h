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

#pragma push_macro("assert")
#undef assert
#define assert(x)
    inline idx_t index(smallidx_t stridesidx __attribute__((unused)), smallidx_t i) const {
        assert(stridesidx==ndim-1);
        return i;
    }

    template<typename ... Types>
    inline idx_t index(smallidx_t stridesidx, smallidx_t i, Types... rest) const {
        assert(stridesidx<ndim);
        return i*strides[stridesidx]+index(stridesidx+1,rest...);
    }

    inline idx_t fill_strides(smallidx_t stridesidx __attribute__((unused))) const {
        assert(stridesidx==ndim-1);
        return 1;
    }

    template<typename ... Types>
    inline idx_t fill_strides(smallidx_t stridesidx, smallidx_t i, Types... rest) const {
        assert(stridesidx<ndim-1);
        return strides[stridesidx]=fill_strides(stridesidx+1,rest...)*i;
    }
#undef assert
#pragma pop_macro("assert")
public:
    template<typename ... Types>
    MultiArray(smallidx_t nfirst, Types... counts) : strides(nullptr) {
        ndim=sizeof...(counts)+1;
        if(ndim>1) {
            strides=new idx_t[ndim-1];
            fill_strides(0,counts...);
            size=nfirst*strides[0];
        } else
            size=nfirst;
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
