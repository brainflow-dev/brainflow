// oscpp library
//
// Copyright (c) 2004-2013 Stefan Kersten <sk@k-hornz.de>
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
//
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifndef OSCPP_STREAM_HPP_INCLUDED
#define OSCPP_STREAM_HPP_INCLUDED

#include <oscpp/detail/host.hpp>
#include <oscpp/error.hpp>
#include <oscpp/types.hpp>
#include <oscpp/util.hpp>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>

namespace OSCPP {

class Stream
{
public:
    Stream()
    {
        m_begin = m_end = m_pos = 0;
    }

    Stream(void* data, size_t size)
    {
        m_begin = static_cast<char*>(data);
        m_end = m_begin + size;
        m_pos = m_begin;
    }

    Stream(const Stream& stream)
    {
        m_begin = m_pos = stream.m_pos;
        m_end = stream.m_end;
    }

    Stream(const Stream& stream, size_t size)
    {
        m_begin = m_pos = stream.m_pos;
        m_end = m_begin + size;
        if (m_end > stream.m_end)
            throw UnderrunError();
    }

    void reset()
    {
        m_pos = m_begin;
    }

    const char* begin() const
    {
        return m_begin;
    }

    char* begin()
    {
        return m_begin;
    }

    const char* end() const
    {
        return m_end;
    }

    size_t capacity() const
    {
        return end() - begin();
    }

    const char* pos() const
    {
        return m_pos;
    }

    char* pos()
    {
        return m_pos;
    }

    void setPos(char* pos)
    {
        assert((pos >= m_begin) && (pos <= m_end));
        m_pos = pos;
    }

    void advance(size_t n)
    {
        m_pos += n;
    }

    bool atEnd() const
    {
        return pos() == end();
    }

    size_t consumed() const
    {
        return pos() - begin();
    }

    size_t consumable() const
    {
        return end() - pos();
    }

    inline void checkAlignment(size_t n) const
    {
        OSCPP::checkAlignment(pos(), n);
    }

protected:
    char* m_begin;
    char* m_end;
    char* m_pos;
};

template <ByteOrder B> class BasicWriteStream : public Stream
{
public:
    BasicWriteStream()
    : Stream()
    {}

    BasicWriteStream(void* data, size_t size)
    : Stream(data, size)
    {}

    BasicWriteStream(const BasicWriteStream& stream)
    : Stream(stream)
    {}

    BasicWriteStream(const BasicWriteStream& stream, size_t size)
    : Stream(stream, size)
    {}

    // throw (OverflowError)
    inline void checkWritable(size_t n) const
    {
        if (consumable() < n)
            throw OverflowError(n - consumable());
    }

    void skip(size_t n)
    {
        checkWritable(n);
        advance(n);
    }

    void zero(size_t n)
    {
        checkWritable(n);
        std::memset(m_pos, 0, n);
        advance(n);
    }

    void putChar(char c)
    {
        checkWritable(1);
        *pos() = c;
        advance(1);
    }

    void putInt32(int32_t x)
    {
        checkWritable(4);
        checkAlignment(4);
        uint32_t uh;
        memcpy(&uh, &x, 4);
        const uint32_t un = convert32<B>(uh);
        std::memcpy(pos(), &un, 4);
        advance(4);
    }

    void putUInt64(uint64_t x)
    {
        checkWritable(8);
        const uint64_t un = convert64<B>(x);
        std::memcpy(pos(), &un, 8);
        advance(8);
    }

    void putFloat32(float f)
    {
        checkWritable(4);
        checkAlignment(4);
        uint32_t uh;
        std::memcpy(&uh, &f, 4);
        const uint32_t un = convert32<B>(uh);
        std::memcpy(pos(), &un, 4);
        advance(4);
    }

    void putFloat64(double f)
    {
        checkWritable(8);
        checkAlignment(4);
        uint64_t uh;
        std::memcpy(&uh, &f, 8);
        const uint64_t un = convert64<B>(uh);
        std::memcpy(pos(), &un, 8);
        advance(8);
    }

    void putData(const void* data, size_t size)
    {
        const size_t padding = OSCPP::padding(size);
        const size_t n = size + padding;
        checkWritable(n);
        std::memcpy(pos(), data, size);
        std::memset(pos() + size, 0, padding);
        advance(n);
    }

    void putString(const char* s)
    {
        putData(s, strlen(s) + 1);
    }
};

typedef BasicWriteStream<NetworkByteOrder> WriteStream;

template <ByteOrder B> class BasicReadStream : public Stream
{
public:
    BasicReadStream()
    {}

    BasicReadStream(const void* data, size_t size)
    : Stream(const_cast<void*>(data), size)
    {}

    BasicReadStream(const BasicReadStream& stream)
    : Stream(stream)
    {}

    BasicReadStream(const BasicReadStream& stream, size_t size)
    : Stream(stream, size)
    {}

    // throw (UnderrunError)
    void checkReadable(size_t n) const
    {
        if (consumable() < n)
            throw UnderrunError();
    }

    // throw (UnderrunError)
    void skip(size_t n)
    {
        checkReadable(n);
        advance(n);
    }

    // throw (UnderrunError)
    inline char peekChar() const
    {
        checkReadable(1);
        return *pos();
    }

    // throw (UnderrunError)
    inline char getChar()
    {
        const char x = peekChar();
        advance(1);
        return x;
    }

    // throw (UnderrunError)
    inline int32_t peekInt32() const
    {
        checkReadable(4);
        checkAlignment(4);
        uint32_t un;
        std::memcpy(&un, pos(), 4);
        const uint32_t uh = convert32<B>(un);
        int32_t        x;
        std::memcpy(&x, &uh, 4);
        return x;
    }

    // throw (UnderrunError)
    inline int32_t getInt32()
    {
        const int32_t x = peekInt32();
        advance(4);
        return x;
    }

    // throw (UnderrunError)
    inline uint64_t getUInt64()
    {
        checkReadable(8);
        uint64_t un;
        std::memcpy(&un, pos(), 8);
        advance(8);
        return convert64<B>(un);
    }

    // throw (UnderrunError)
    inline float getFloat32()
    {
        checkReadable(4);
        checkAlignment(4);
        uint32_t un;
        std::memcpy(&un, pos(), 4);
        advance(4);
        const uint32_t uh = convert32<B>(un);
        float          f;
        std::memcpy(&f, &uh, 4);
        return f;
    }

    // throw (UnderrunError)
    inline double getFloat64()
    {
        checkReadable(8);
        checkAlignment(4);
        uint64_t un;
        std::memcpy(&un, pos(), 8);
        advance(8);
        const uint64_t uh = convert64<B>(un);
        double         f;
        std::memcpy(&f, &uh, 8);
        return f;
    }

    // throw (UnderrunError, ParseError)
    const char* getString()
    {
        checkReadable(4); // min string length

        const char* ptr = static_cast<const char*>(pos()) + 3;
        const char* end = static_cast<const char*>(this->end());

        while (true)
        {
            if (ptr >= end)
                throw UnderrunError();
            if (*ptr == '\0')
                break;
            ptr += 4;
        }

        const char* x = pos();
        advance(ptr - pos() + 1);

        return x;
    }
};

typedef BasicReadStream<NetworkByteOrder> ReadStream;
} // namespace OSCPP

#endif // OSCPP_STREAM_HPP_INCLUDED
