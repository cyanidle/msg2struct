#pragma once
#ifndef MSG2STRUCT_HPP
#define MSG2STRUCT_HPP

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <endian.h>

#ifndef MSG_2_STRUCT_FIXEXT //support for fixext msgpack types
#define MSG_2_STRUCT_FIXEXT 1
#endif

#define _ALWAYS_INLINE __attribute__((always_inline))
#define _FLATTEN __attribute__((flatten))

namespace msg2struct
{

enum class Headers {
    never_used, //  11000001 	0xc1
    invalid = never_used,
// Fix (inline size or data)
    fixuint,    //  0xxxx       0xe0 - 0xff,
    fixint,     //  111xxxx     0x00 - 0x7f,
    fixmap,     // 	1000xxxx 	0x80 - 0x8f
    fixarray,   // 	1001xxxx 	0x90 - 0x9f
    fixstr,     // 	101xxxxx 	0xa0 - 0xbf
// Trivial
    nil,        // 	11000000 	0xc0
    hfalse,     // 	11000010 	0xc2
    htrue,      // 	11000011 	0xc3
// Floats
    float32,    // 	11001010 	0xca
    float64,    // 	11001011 	0xcb
// Unsigned Ints
    uint8,      // 	11001100 	0xcc
    uint16,     // 	11001101 	0xcd
    uint32,     // 	11001110 	0xce
    uint64,     // 	11001111 	0xcf
// Ints
    int8,       // 	11010000 	0xd0
    int16,      // 	11010001 	0xd1
    int32,      // 	11010010 	0xd2
    int64,      // 	11010011 	0xd3
// Containers
    array16,    // 	11011100 	0xdc
    array32,    // 	11011101 	0xdd
    map16,      // 	11011110 	0xde
    map32,      // 	11011111 	0xdf
// Fixext
    fixext1,    // 	11010100 	0xd4
    fixext2,    // 	11010101 	0xd5
    fixext4,    // 	11010110 	0xd6
    fixext8,    // 	11010111 	0xd7
    fixext16,   // 	11011000 	0xd8
// Composites
    bin8,       // 	11000100 	0xc4
    bin16,      // 	11000101 	0xc5
    bin32,      // 	11000110 	0xc6

    ext8,       // 	11000111 	0xc7
    ext16,      // 	11001000 	0xc8
    ext32,      // 	11001001 	0xc9

    str8,       // 	11011001 	0xd9
    str16,      // 	11011010 	0xda
    str32,      // 	11011011 	0xdb
};

struct Binary {
    const unsigned char* data{};
    size_t size{};

    explicit operator bool() const noexcept {
        return data;
    }
};

struct String {
    const char* str{};
    size_t size{};

    explicit operator bool() const noexcept {
        return str;
    }
};

namespace impl {


inline Headers header(unsigned char byte) {
    switch (byte) {
    case 0xc0: return Headers::nil;
    case 0xc1: return Headers::never_used;
    case 0xc2: return Headers::hfalse;
    case 0xc3: return Headers::htrue;
    case 0xc4: return Headers::bin8;
    case 0xc5: return Headers::bin16;
    case 0xc6: return Headers::bin32;
    case 0xc7: return Headers::ext8;
    case 0xc8: return Headers::ext16;
    case 0xc9: return Headers::ext32;
    case 0xca: return Headers::float32;
    case 0xcb: return Headers::float64;
    case 0xcc: return Headers::uint8;
    case 0xcd: return Headers::uint16;
    case 0xce: return Headers::uint32;
    case 0xcf: return Headers::uint64;
    case 0xd0: return Headers::int8;
    case 0xd1: return Headers::int16;
    case 0xd2: return Headers::int32;
    case 0xd3: return Headers::int64;
    case 0xd4: return Headers::fixext1;
    case 0xd5: return Headers::fixext2;
    case 0xd6: return Headers::fixext4;
    case 0xd7: return Headers::fixext8;
    case 0xd8: return Headers::fixext16;
    case 0xd9: return Headers::str8;
    case 0xda: return Headers::str16;
    case 0xdb: return Headers::str32;
    case 0xdc: return Headers::array16;
    case 0xdd: return Headers::array32;
    case 0xde: return Headers::map16;
    case 0xdf: return Headers::map32;
    default:
        if ((byte & (1 << 7)) == 0) return Headers::fixint;
        if ((byte & (7 << 5)) == (7 << 5)) return Headers::fixuint;
        if ((byte & (15 << 4)) == (1 << 7)) return Headers::fixmap;
        if ((byte & (15 << 4)) == (9 << 4)) return Headers::fixarray;
        if ((byte & (7 << 5)) == (5 << 5)) return Headers::fixstr;
        abort();
    }
}

inline size_t headerSizeof(Headers type) {
    switch (type) {
    default:
        return 1; //trivial 1 byte (inline or fix*)
    case Headers::bin8:
    case Headers::ext8:
    case Headers::str8:
    case Headers::fixext1:
        return 2; //type + trivial 1-byte
    case Headers::bin16:
    case Headers::ext16:
    case Headers::uint16:
    case Headers::int16:
    case Headers::str16:
    case Headers::array16:
    case Headers::map16:
    case Headers::fixext2:
        return 3; //type + trivial 2-byte
    case Headers::str32:
    case Headers::array32:
    case Headers::bin32:
    case Headers::ext32:
    case Headers::map32:
    case Headers::float32:
    case Headers::uint32:
    case Headers::int32:
    case Headers::fixext4:
        return 5; //type + trivial 4-byte
    case Headers::uint64:
    case Headers::int64:
    case Headers::float64:
    case Headers::fixext8:
        return 9; //type + trivial 8-byte
    case Headers::fixext16:
        return 17; //type + trivial 16-byte
    }
}

template<typename T>
void getTrivial(T& v, const unsigned char* data) {
    memcpy(&v, data, sizeof(v));
    if (sizeof(T) == 2) v = T(be16toh(uint16_t(v)));
    if (sizeof(T) == 4) v = T(be32toh(uint32_t(v)));
    if (sizeof(T) == 8) v = T(be64toh(uint64_t(v)));
}

template<typename T, typename U>
bool getTrivialWith(U& out, const unsigned char* data, size_t size) {
    T v;
    if (size <= sizeof(v)) return false;
    impl::getTrivial(v, data);
    out = T(v);
    return true;
}

template<typename SizeT>
void getComposite(Binary& result, const unsigned char* data, size_t size, size_t add) {
    uint32_t sz;
    if (size <= sizeof(sz) + add) return;
    impl::getTrivial(sz, data);
    if (size <= sizeof(sz) + sz + add) return;
    result.data = data + sizeof(sz) + 1 + add;
    result.size = sz;
}

} //impl

class Iterator {
    const unsigned char* data;
    size_t size;
    Headers type;
public:
    const unsigned char* Data() const noexcept {
        return data;
    }

    Headers Type() const noexcept {
        return type;
    }

    Iterator() noexcept : type(Headers::invalid) {}

    static Iterator InvalidAt(const unsigned char* point) noexcept {
        Iterator it;
        it.data = point;
        return it;
    }

    Iterator(const unsigned char* _data, size_t _size) noexcept : data(_data), size(_size) {
        type = _size ? impl::header(_data[0]) : Headers::invalid;
    }

    _FLATTEN Binary GetComposite() const noexcept {
        Binary result;
        bool isExt = type >= Headers::ext8 && type <= Headers::ext32;
        switch(type) {
        case Headers::ext32:
        case Headers::bin32:
        case Headers::str32: {
            impl::getComposite<uint32_t>(result, data, size, isExt);
            return result;
        }
        case Headers::ext16:
        case Headers::bin16:
        case Headers::str16: {
            impl::getComposite<uint16_t>(result, data, size, isExt);
            return result;
        }
        case Headers::ext8:
        case Headers::bin8:
        case Headers::str8: {
            impl::getComposite<uint8_t>(result, data, size, isExt);
            return result;
        }
#if MSG_2_STRUCT_FIXEXT
        //TODO
#endif
        case Headers::fixstr: {
            size_t len = data[0] & 31;
            if (size <= len) return result;
            result.data = data + 1;
            result.size = len;
            return result;
        }
        default:
            return result;
        }
    }

    template<typename T>
    _FLATTEN bool GetUnsigned(T& out) const noexcept {
        switch(type) {
        case Headers::fixuint: {
            out = T(uint8_t(data[0]));
            return true;
        }
        case Headers::uint8: return impl::getTrivialWith<uint8_t>(out, data, size);
        case Headers::uint16: return impl::getTrivialWith<uint16_t>(out, data, size);
        case Headers::uint32: return impl::getTrivialWith<uint32_t>(out, data, size);
        case Headers::uint64: return impl::getTrivialWith<uint64_t>(out, data, size);
        default: return false;
        }
    }

    template<typename T>
    _FLATTEN bool GetSigned(T& out) const noexcept {
        switch(type) {
        case Headers::fixint: {
            out = T(int8_t(data[0]));
            return true;
        }
        case Headers::int8: return impl::getTrivialWith<int8_t>(out, data, size);
        case Headers::int16: return impl::getTrivialWith<int16_t>(out, data, size);
        case Headers::int32: return impl::getTrivialWith<int32_t>(out, data, size);
        case Headers::int64: return impl::getTrivialWith<int64_t>(out, data, size);
        default: return false;
        }
    }

    template<typename T>
    _FLATTEN bool GetFloat(T& out) const noexcept {
        switch(type) {
        case Headers::float32: return impl::getTrivialWith<float>(out, data, size);
        case Headers::float64: return impl::getTrivialWith<double>(out, data, size);
        default: return false;
        }
    }

    size_t GetArraySize() const noexcept {
        switch(type) {
        case Headers::array16: {
            uint16_t sz;
            if (size <= 2) return size_t(-1);
            impl::getTrivial(sz, data);
            return size_t(sz);
        }
        case Headers::array32: {
            uint32_t sz;
            if (size <= 4) return size_t(-1);
            impl::getTrivial(sz, data);
            return size_t(sz);
        }
        case Headers::fixarray: {
            return size_t(data[0] & 15);
        }
        default:
            return size_t(-1);
        }
    }

    bool IsValid() const noexcept {
        return type != Headers::invalid;
    }

    bool Next() noexcept {
        if (!IsValid()) return false;
        auto step = impl::headerSizeof(type) + GetComposite().size;
        if (step >= size) {
            type = Headers::invalid;
            return false;
        }
        size -= step;
        data += step;
        type = impl::header(data[0]);
        return IsValid();
    }

    Iterator Peek() const noexcept {
        Iterator temp = *this;
        temp.Next();
        return temp;
    }
};

namespace impl {

template<typename Fn, typename...Args>
_ALWAYS_INLINE void apply(Fn& f, Args&&...args) {
    int _helper[] = {(f(args), 0)...};
    (void)_helper;
}

#define MSG_2_STRUCT(...) \
static constexpr int is_msg_2_struct = 1; \
template<typename Fn> void _msg2struct(Fn&& _) const {msg2struct::impl::apply(_, __VA_ARGS__);} \
template<typename Fn> void _msg2struct(Fn&& _) {msg2struct::impl::apply(_, __VA_ARGS__);}

#define MSG_2_STRUCT_INHERIT(parent, ...) \
template<typename Fn> void _msg2struct(Fn&& _) const {parent::_msg2struct(_); msg2struct::impl::apply(_, __VA_ARGS__);} \
template<typename Fn> void _msg2struct(Fn&& _) {parent::_msg2struct(_); msg2struct::impl::apply(_, __VA_ARGS__);}

template<typename T>
struct ParseHelper {
    Iterator it;
    bool err;
    template<typename U>
    void operator()(U& field) {
        if (!it.IsValid()) return;
        if (!Parse(field, it)) {
            err = true;
            it = {};
        } else {
            it.Next();
        }
    }
};
}

// For now only tuple-like parse and dump

template<typename T, int = T::is_msg_2_struct>
bool Parse(T& object, Iterator it) {
    auto helper = impl::ParseHelper<T>{it, false};
    object._msg2struct(helper);
    return !helper.err;
}

#define _PARSE_WITH(method, T) \
inline bool Parse(T& i, Iterator it) { \
    return it.method(i);  \
}

_PARSE_WITH(GetSigned, int8_t)
_PARSE_WITH(GetSigned, int16_t)
_PARSE_WITH(GetSigned, int32_t)
_PARSE_WITH(GetSigned, int64_t)

_PARSE_WITH(GetUnsigned, uint8_t)
_PARSE_WITH(GetUnsigned, uint16_t)
_PARSE_WITH(GetUnsigned, uint32_t)
_PARSE_WITH(GetUnsigned, uint64_t)

_PARSE_WITH(GetFloat, float)
_PARSE_WITH(GetFloat, double)

inline bool Parse(String& i, Iterator it) {
    auto bin = it.GetComposite();
    switch (it.Type()) {
    case Headers::fixstr:
    case Headers::str8:
    case Headers::str16:
    case Headers::str32:
        i.size = bin.size;
        i.str = (const char*)bin.data;
        return bool(i);
    default:
        return false;
    }
}

inline bool Parse(Binary& i, Iterator it) {
    i = it.GetComposite();
    return bool(i);
}

} //msg2struct

#endif //MSG2STRUCT_HPP
