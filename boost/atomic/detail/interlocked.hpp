#ifndef BOOST_DETAIL_ATOMIC_INTERLOCKED_HPP
#define BOOST_DETAIL_ATOMIC_INTERLOCKED_HPP

//  Copyright (c) 2009 Helge Bahmann
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <boost/detail/interlocked.hpp>

#include <boost/atomic/detail/base.hpp>
#include <boost/atomic/detail/builder.hpp>

namespace boost {
namespace detail {
namespace atomic {

static inline void full_fence(void)
{
    long tmp;
    BOOST_INTERLOCKED_EXCHANGE(&tmp, 0);
}

template<>
inline void platform_atomic_thread_fence(memory_order order)
{
    switch(order) {
        case memory_order_seq_cst:
            full_fence();
        default:;
    }
}

static inline void fence_after_load(memory_order order)
{
    switch(order) {
        case memory_order_seq_cst:
            full_fence();
        case memory_order_acquire:
        case memory_order_acq_rel:
        default:;
    }
}


template<typename T>
class atomic_interlocked_32 {
public:
    explicit atomic_interlocked_32(T v) : i(v) {}
    atomic_interlocked_32() {}
    T load(memory_order order=memory_order_seq_cst) const volatile
    {
        T v=*reinterpret_cast<volatile const T *>(&i);
        fence_after_load(order);
        return v;
    }
    void store(T v, memory_order order=memory_order_seq_cst) volatile
    {
        if (order!=memory_order_seq_cst) {
            *reinterpret_cast<volatile T *>(&i)=v;
        } else {
            exchange(v);
        }
    }
    bool compare_exchange_strong(
        T &expected,
        T desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        T prev=expected;
        expected=(T)BOOST_INTERLOCKED_COMPARE_EXCHANGE((long *)(&i), (long)desired, (long)expected);
        bool success=(prev==expected);
        return success;
    }
    bool compare_exchange_weak(
        T &expected,
        T desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        return compare_exchange_strong(expected, desired, success_order, failure_order);
    }
    T exchange(T r, memory_order order=memory_order_seq_cst) volatile
    {
        return (T)BOOST_INTERLOCKED_EXCHANGE((long *)&i, (long)r);
    }
    T fetch_add(T c, memory_order order=memory_order_seq_cst) volatile
    {
        return (T)BOOST_INTERLOCKED_EXCHANGE_ADD((long *)&i, c);
    }
    
    bool is_lock_free(void) const volatile {return true;}
    
    typedef T integral_type;
private:
    T i;
};

# if defined(_M_IA64) || defined(_M_AMD64)

#if defined( BOOST_USE_WINDOWS_H )

# include <windows.h>

# define BOOST_INTERLOCKED_EXCHANGE_ADD64 InterlockedExchangeAdd64
# define BOOST_INTERLOCKED_EXCHANGE64 InterlockedExchange64
# define BOOST_INTERLOCKED_COMPARE_EXCHANGE64 InterlockedCompareExchange64

#else

extern "C" boost::int64_t __cdecl _InterlockedExchangeAdd64(boost::int64_t volatile *, boost::int64_t);
extern "C" boost::int64_t __cdecl _InterlockedExchange64(boost::int64_t volatile *, boost::int64_t);
extern "C" boost::int64_t __cdecl _InterlockedCompareExchange64(boost::int64_t volatile *, boost::int64_t, boost::int64_t);

# pragma intrinsic( _InterlockedExchangeAdd64 )
# pragma intrinsic( _InterlockedExchange64 )
# pragma intrinsic( _InterlockedCompareExchange64 )

# define BOOST_INTERLOCKED_EXCHANGE_ADD64 _InterlockedExchangeAdd64
# define BOOST_INTERLOCKED_EXCHANGE64 _InterlockedExchange64
# define BOOST_INTERLOCKED_COMPARE_EXCHANGE64 _InterlockedCompareExchange64

#endif

template<typename T>
__declspec(align(64)) class atomic_interlocked_64 {
public:
    explicit atomic_interlocked_64(T v) : i(v) {}
    atomic_interlocked_64() {}
    T load(memory_order order=memory_order_seq_cst) const volatile
    {
        T v=*reinterpret_cast<volatile const T *>(&i);
        fence_after_load(order);
        return v;
    }
    void store(T v, memory_order order=memory_order_seq_cst) volatile
    {
        if (order!=memory_order_seq_cst) {
            *reinterpret_cast<volatile T *>(&i)=v;
        } else {
            exchange(v);
        }
    }
    bool compare_exchange_strong(
        T &expected,
        T desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        T prev=expected;
        expected=(T)BOOST_INTERLOCKED_COMPARE_EXCHANGE64((boost::int64_t *)(&i), (boost::int64_t)desired, (boost::int64_t)expected);
        bool success=(prev==expected);
        return success;
    }
    bool compare_exchange_weak(
        T &expected,
        T desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        return compare_exchange_strong(expected, desired, success_order, failure_order);
    }
    T exchange(T r, memory_order order=memory_order_seq_cst) volatile
    {
        return (T)BOOST_INTERLOCKED_EXCHANGE64((boost::int64_t *)&i, (boost::int64_t)r);
    }
    T fetch_add(T c, memory_order order=memory_order_seq_cst) volatile
    {
      return (T)BOOST_INTERLOCKED_EXCHANGE_ADD64((boost::int64_t *)&i, c);
    }
    
    bool is_lock_free(void) const volatile {return true;}
    
    typedef T integral_type;
private:
    T i;
};
#endif

#if BOOST_MSVC >= 1500 && (defined(_M_IA64) || defined(_M_AMD64)) && defined(BOOST_ATOMIC_HAVE_SSE2)

#if defined( BOOST_USE_WINDOWS_H )

# include <windows.h>

# define BOOST_INTERLOCKED_COMPARE_EXCHANGE128 InterlockedCompareExchange128

#else

extern "C" unsigned char __cdecl
_InterlockedCompareExchange128(boost::int64_t volatile*, boost::int64_t, boost::int64_t, boost::int64_t*);

# pragma intrinsic( _InterlockedCompareExchange128 )

# define BOOST_INTERLOCKED_COMPARE_EXCHANGE128 _InterlockedCompareExchange128

#endif

#include <emmintrin.h>

template<typename T>
__declspec(align(128)) class atomic_interlocked_128 {
public:
    explicit atomic_interlocked_128(T v) : i(v) {}
    atomic_interlocked_128() {}
    T load(memory_order order=memory_order_seq_cst) const volatile
    {
        T v;
        if (order!=memory_order_seq_cst) {
          v = _mm_load_si128((__m128i)(&i));
        } else {
          v = *reinterpret_cast<volatile const T *>(&i);
        }
        fence_after_load(order);
        return v;
    }

    void store(T v, memory_order order=memory_order_seq_cst) volatile
    {
        if (order!=memory_order_seq_cst) {
            *reinterpret_cast<volatile T *>(&i)=v;
        } else {
            _mm_storeu_si128((__m128i)(&i), v);
        }
    }

    bool compare_exchange_strong(
        T &expected,
        T desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        boost::int64_t* expected_raw = &expected;
        boost::int64_t* desired_raw = &desired;
        volatile boost::int64_t* i_raw = &i;
        T prev = i;
        bool success=BOOST_INTERLOCKED_COMPARE_EXCHANGE128
            (i_raw, desired_raw[1], desired_raw[0], expected_raw);
        expected = prev;
        return success;
    }
    bool compare_exchange_weak(
        T &expected,
        T desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        return compare_exchange_strong(expected, desired, success_order, failure_order);
    }
    T exchange(T r, memory_order order=memory_order_seq_cst) volatile
    {
        boost::int64_t* desired = &r;

        while (!BOOST_INTERLOCKED_COMPARE_EXCHANGE128
            ((volatile boost::int64_t*)(&i), desired[1], desired[0],
             (boost::int64_t*)&i))
        {};

        return r;
    }
    T fetch_add(T c, memory_order order=memory_order_seq_cst) volatile
    {
        T expected = i, desired;

        do {
            desired = expected + c;
        } while(!compare_exchange_strong(expected, desired, order, memory_order_relaxed));
        return expected;
    }
    
    bool is_lock_free(void) const volatile {return true;}
    
    typedef T integral_type;
private:
    T i;
};

template<typename T>
class platform_atomic_integral<T, 16> : public build_atomic_from_add<atomic_interlocked_128<T> >{
public:
    typedef build_atomic_from_add<atomic_interlocked_128<T> > super;
    explicit platform_atomic_integral(T v) : super(v) {}
    platform_atomic_integral(void) {}
};
#endif

template<typename T>
class platform_atomic_integral<T, 4> : public build_atomic_from_add<atomic_interlocked_32<T> > {
public:
    typedef build_atomic_from_add<atomic_interlocked_32<T> > super;
    explicit platform_atomic_integral(T v) : super(v) {}
    platform_atomic_integral(void) {}
};

template<typename T>
class platform_atomic_integral<T, 1>: public build_atomic_from_larger_type<atomic_interlocked_32<uint32_t>, T> {
public:
    typedef build_atomic_from_larger_type<atomic_interlocked_32<uint32_t>, T> super;
    
    explicit platform_atomic_integral(T v) : super(v) {}
    platform_atomic_integral(void) {}
};

template<typename T>
class platform_atomic_integral<T, 2>: public build_atomic_from_larger_type<atomic_interlocked_32<uint32_t>, T> {
public:
    typedef build_atomic_from_larger_type<atomic_interlocked_32<uint32_t>, T> super;
    
    explicit platform_atomic_integral(T v) : super(v) {}
    platform_atomic_integral(void) {}
};

# if defined(_M_IA64) || defined(_M_AMD64)
template<typename T>
class platform_atomic_integral<T, 8>: public build_atomic_from_add<atomic_interlocked_64<uint64_t> > {
public:
    typedef build_atomic_from_add<atomic_interlocked_64<uint64_t> > super;
    
    explicit platform_atomic_integral(T v) : super(v) {}
    platform_atomic_integral(void) {}
};

template<>
class platform_atomic_integral<void*, 8>: public build_atomic_from_add<atomic_interlocked_64<void*> > {
public:
    typedef build_atomic_from_add<atomic_interlocked_64<void*> > super;
    
    explicit platform_atomic_integral(void* v) : super(v) {}
    platform_atomic_integral(void) {}
};
#endif

}
}
}

#endif
