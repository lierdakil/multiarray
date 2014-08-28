#ifndef MULTIARRAY_H
#define MULTIARRAY_H

#include <iterator>
#include <memory>
#include <array>
#include <stdexcept>

template<typename T, unsigned int ndim>
class MultiArray
{
private:
    typedef unsigned long long int idx_t;
    typedef unsigned int smallidx_t;

    std::shared_ptr<idx_t> strides;
    idx_t arr_size;
    std::shared_ptr<T> data;

    inline idx_t index(smallidx_t, smallidx_t i) const {
        return i;
    }

    template<typename ... Types>
    inline idx_t index(smallidx_t stridesidx, smallidx_t i, Types... rest) const {
        return i*strides.get()[stridesidx]+index(stridesidx+1,rest...);
    }

    inline idx_t fill_strides(smallidx_t stridesidx,smallidx_t i) const {
        return strides.get()[stridesidx]=i;
    }

    template<typename ... Types>
    inline idx_t fill_strides(smallidx_t stridesidx, smallidx_t i, Types... rest) const {
        return strides.get()[stridesidx]=fill_strides(stridesidx+1,rest...)*i;
    }

    inline const T& operator[](idx_t idx) const {
        check_size(idx);
        return data.get()[idx];
    }

    inline T& operator[](idx_t idx) {
        check_size(idx);
        if(!data.unique()) {
            std::shared_ptr<T> other(new T[arr_size],std::default_delete<T[]>());
            std::copy(data.get(),data.get()+arr_size,other.get());
            data.swap(other);
        }
        return data.get()[idx];
    }

    inline void check_valid() const {
        if(!valid())
            throw std::logic_error("Using invalid MultiArray");
    }

    inline void check_size(idx_t idx) const {
        if(idx>=arr_size)
            throw std::out_of_range("MultiArray index out of range");
    }

    template<smallidx_t ...S> struct seq {};
    template<smallidx_t N, smallidx_t ...S> struct gens : gens<N-1, N-1, S...> {};
    template<smallidx_t ...S> struct gens<0, S...>{ typedef seq<S...> type; };

    typedef typename gens<ndim>::type idxseq;

    typedef std::array<smallidx_t,ndim> multiIdx_t;

    //array-based helpers
    template<typename A, smallidx_t ... I>
    inline const T& get_impl(const A& arr, seq<I...>) const {
        return get(arr[I]...);
    }

    template<typename A, smallidx_t ... I>
    inline T& set_impl(const A& arr, seq<I...>) {
        return set(arr[I]...);
    }

    const multiIdx_t msize;

public:
    template<typename ... Types>
    MultiArray(smallidx_t nfirst, Types... counts) :
        strides(new idx_t[ndim-1],std::default_delete<idx_t[]>()),
        arr_size(nfirst*fill_strides(0,counts...)),
        data(new T[arr_size],std::default_delete<T[]>()),
        msize{nfirst,counts...}
    {
        static_assert(ndim==sizeof...(counts)+1,"Invalid number of arguments in MultiArray constructor");
    }

    MultiArray(smallidx_t nfirst) :
        strides(nullptr),
        arr_size(nfirst),
        data(new T[arr_size],std::default_delete<T[]>()),
        msize{nfirst}
    {
        static_assert(ndim==1,"Invalid number of arguments in MultiArray constructor");
    }

    MultiArray(const MultiArray &other) :
        strides(other.strides),
        arr_size(other.arr_size),
        data(other.data),
        msize(other.msize)
    {
    }

    MultiArray(MultiArray &&other) :
        strides(other.strides),
        arr_size(other.arr_size),
        data(other.data),
        msize(other.msize)
    {
        other.arr_size=0;
        other.strides.reset();
        other.data.reset();
    }

    //arg-based
    template<typename ... Types>
    inline const T& get(Types... indexes) const {
        static_assert(sizeof...(indexes)==ndim,"Invalid number of arguments in MultiArray::get(...)");
        check_valid();
        return (*this)[index(0,indexes...)];
    }

    template<typename ... Types>
    inline T& set(Types... indexes) {
        static_assert(sizeof...(indexes)==ndim,"Invalid number of arguments in MultiArray::set(...)");
        check_valid();
        return (*this)[index(0,indexes...)];
    }

    template<typename ... Types>
    inline const T& operator()(Types... indexes) const {
        return get(indexes...);
    }

    template<typename ... Types>
    inline T& operator()(Types... indexes) {
        return set(indexes...);
    }

    //array-based

    inline const T& get(const multiIdx_t& arr) const {
        return get_impl(arr,idxseq());
    }

    inline T& set(const multiIdx_t& arr) {
        return set_impl(arr,idxseq());
    }

    inline const T& operator()(const multiIdx_t &arr) const {
        return get(arr);
    }

    inline T& operator()(const multiIdx_t &arr) {
        return set(arr);
    }

    inline multiIdx_t size() const {
        return msize;
    }

    inline bool valid() const {
        return (strides||ndim==1) && data && arr_size;
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
        T& operator*() {arr->check_valid(); return (*arr)[idx];}
        T* operator->() {return &(**this);}

        MultiArray* parent() { return arr; }
        const multiIdx_t index() const {
            arr->check_valid();
            std::array<smallidx_t,ndim> i;
            auto t=idx;
            for(smallidx_t j=0; j<ndim-1; ++j) {
                i[j]=t/arr->strides.get()[j];
                t=t%arr->strides.get()[j];
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
        const T& operator*() const {arr->check_valid(); return (*arr)[idx];}
        const T* operator->() const {return &(**this);}

        const MultiArray* parent() const { return arr; }
        const multiIdx_t index() const {
            arr->check_valid();
            std::array<smallidx_t,ndim> i;
            auto t=idx;
            for(smallidx_t j=0; j<ndim-1; ++j) {
                i[j]=t/arr->strides.get()[j];
                t=t%arr->strides.get()[j];
            }
            i[ndim-1]=t;
            return i;
        }
    };

    iterator begin() {
        return iterator(this,0);
    }

    iterator end() {
        return iterator(this,arr_size);
    }

    template<typename ... Types>
    iterator make_iterator(Types... indices) {
        return iterator(this, index(0,indices...));
    }

    const_iterator const_begin() const {
        return const_iterator(this,0);
    }

    const_iterator const_end() const {
        return const_iterator(this,arr_size);
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
