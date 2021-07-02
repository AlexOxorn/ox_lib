#include <ox/bytes.h>

namespace ox {
    void swap(void* data, int size, int length) {
        for(int offset = 0; offset < length; offset++) {
            switch(size) {
            case sizeof(u16):
                ox::swap(reinterpret_cast<u16*>(data) + offset);
                break;
            case sizeof(u32):
                ox::swap(reinterpret_cast<u32*>(data) + offset);
                break;
            case sizeof(u64):
                ox::swap(reinterpret_cast<u64*>(data) + offset);
                break;
            }
        }
    }

}
