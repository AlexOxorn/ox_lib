#ifndef _OXORN_BYTES_H
#define _OXORN_BYTES_H

#include <stddef.h>
#include <string>
#include <array>
#include <concepts>
#include <ox/types.h>
#include <limits.h>
#include <type_traits>
#include <bit>

namespace ox {
    enum {
        O32_LITTLE_ENDIAN = 0x03020100ul,
        O32_BIG_ENDIAN = 0x00010203ul,
        O32_PDP_ENDIAN = 0x01000302ul,      /* DEC PDP-11 (aka ENDIAN_LITTLE_WORD) */
        O32_HONEYWELL_ENDIAN = 0x02030001ul /* Honeywell 316 (aka ENDIAN_BIG_WORD) */
    };

    template<size_t N> struct integer_of_size {};
    template<> struct integer_of_size<1> { using type = i8; };
    template<> struct integer_of_size<2> { using type = i16; };
    template<> struct integer_of_size<4> { using type = i32; };
    template<> struct integer_of_size<8> { using type = i64; };
    template<size_t N> using integer_of_size_t = typename integer_of_size<N>::type;

    template<size_t N> struct unsigned_of_size {};
    template<> struct unsigned_of_size<1> { using type = u8; };
    template<> struct unsigned_of_size<2> { using type = u16; };
    template<> struct unsigned_of_size<4> { using type = u32; };
    template<> struct unsigned_of_size<8> { using type = u64; };
    template<size_t N> using unsigned_of_size_t = typename unsigned_of_size<N>::type;

    static constexpr u8 o32_host_order[4] =  { 0, 1, 2, 3 };

    constexpr u32 O32_HOST_ORDER = std::bit_cast<u32>(o32_host_order);

    template <std::integral T>
    T bswap(T number) {
        constexpr int size = sizeof(T);
        if constexpr (size == 1) {
            return number;
        } else {
            using UT = std::make_unsigned_t<T>;
            UT result{};
            for(int i = size; i > size/2; i--) {
                UT high_mask = UT{0xff} << ((i - 1) * 8);
                UT low_mask = UT{0xff} << ((size - i) * 8);
                int shift = (i - size/2) * 16 - 8;
                result |= (number & high_mask) >> shift;
                result |= (number & low_mask) << shift;
            }
            return std::bit_cast<T>(result);
        }
    }

    inline u32 rotl(u32 x, int shift) {
        shift &= 31;
        if (!shift) return x;
        return (x << shift) | (x >> (32 - shift));
    }

    inline u32 rotr(u32 x, int shift) {
        shift &= 31;
        if (!shift) return x;
        return (x >> shift) | (x << (32 - shift));
    }

    inline u64 rotl(u64 x, unsigned int shift) {
        unsigned int n = shift % 64;
        return (x << n) | (x >> (64 - n));
    }

    inline u64 rotr(u64 x, unsigned int shift) {
        unsigned int n = shift % 64;
        return (x >> n) | (x << (64 - n));
    }

    inline u8 swap8(u8 data) {return data;}
    inline u32 swap24(const u8* data) {return (data[0] << 16) | (data[1] << 8) | data[2];}

    void swap(void* data, int size, int length = 1);

    template <std::integral T>
    inline T from_big_endian(T data){
        swap<T>(reinterpret_cast<u8*>(&data));
        return data;
    }

    template <std::integral T>
    void bswap(T* data, int length = 1) {
        for(int i = 0; i < length; i++)
            data[i] = bswap(data[i]);
    };

    template<scalar_endianable T> requires (!std::is_integral_v<T>)
    void bswap(T* data, int length = 1) {
        ox::bswap(reinterpret_cast<unsigned_of_size_t<sizeof(T)>*>(data), length);
    }

    template<custom_endianable T>
    void bswap(T* data, int length = 1) {
        for(int offset = 0; offset < length; offset++) {
            data[offset].endian_swap();
        }
    }
}

#endif
