#ifndef _OXORN_BYTES_H
#define _OXORN_BYTES_H

#include <stddef.h>
#include <string>
#include <array>
#include <concepts>
#include <ox/types.h>

namespace ox {
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

	template <std::integral T>
	void swap(T* data) {
		*data = bswap(*data);
	};

    void swap(void* data, int size, int length = 1);

	template <std::integral T>
	inline T from_big_endian(T data){
		swap<T>(reinterpret_cast<u8*>(&data));
		return data;
	}
}

#endif
