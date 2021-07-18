#ifndef _OXORN_BYTES_H
#define _OXORN_BYTES_H

#include <stddef.h>
#include <string>
#include <array>
#include <concepts>
#include <ox/types.h>
#include <limits.h>

namespace ox {
    enum {
        O32_LITTLE_ENDIAN = 0x03020100ul,
        O32_BIG_ENDIAN = 0x00010203ul,
        O32_PDP_ENDIAN = 0x01000302ul,      /* DEC PDP-11 (aka ENDIAN_LITTLE_WORD) */
        O32_HONEYWELL_ENDIAN = 0x02030001ul /* Honeywell 316 (aka ENDIAN_BIG_WORD) */
    };

    static constexpr union {
        u32 value;
        u8 bytes[4];
    } o32_host_order = { .bytes = { 0, 1, 2, 3 } };

    const inline u32 O32_HOST_ORDER = o32_host_order.value;

    template <std::integral T>
    T bswap(T number) {
        constexpr int size = sizeof(T);
        if constexpr (size == 1) {
            return number;
        } else {
            T result{};
            for(int i = size; i > size/2; i--) {
                T high_mask = T{0xff} << ((i - 1) * 8);
                T low_mask = T{0xff} << ((size - i) * 8);
                int shift = (i - size/2) * 16 - 8;
                result |= (number & high_mask) >> shift;
                result |= (number & low_mask) << shift;
            }
            return result;
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
    void swap(T* data, int length = 1) {
        for(int i = 0; i < length; i++)
            data[i] = bswap(data[i]);
    };

    template<scalar_endianable T> requires (!std::is_integral_v<T>)
    void swap(T* data, int length = 1) {
        switch(sizeof(T)) {
        case sizeof(u16):
            ox::swap(reinterpret_cast<u16*>(data), length);
            break;
        case sizeof(u32):
            ox::swap(reinterpret_cast<u32*>(data), length);
            break;
        case sizeof(u64):
            ox::swap(reinterpret_cast<u64*>(data), length);
            break;
        }
    }

    template<custom_endianable T>
    void swap(T* data, int length = 1) {
        for(int offset = 0; offset < length; offset++) {
            data[offset].endian_swap();
        }
    }
}

#endif
