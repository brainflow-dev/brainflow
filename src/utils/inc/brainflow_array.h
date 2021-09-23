#pragma once

#include <algorithm>
#include <array>
#include <iostream>
#include <memory>
#include <stdlib.h>
#include <string.h>

#include "brainflow_constants.h"
#include "brainflow_exception.h"


template <typename T, size_t Dim>
class BrainFlowArray;
template <typename T, size_t Dim>
std::ostream &operator<< (std::ostream &os, const BrainFlowArray<T, Dim> &array);

template <size_t N>
static inline int product (const std::array<int, N> &array)
{
    int result = 1;

    for (int i = 0; i < N; i++)
    {
        result *= array[i];
    }

    return result;
}

template <size_t N>
static inline std::array<int, N> zero_array ()
{
    std::array<int, N> result;
    result.fill (0);
    return result;
}

static inline std::array<int, 1> make_array (int size0)
{
    std::array<int, 1> result;
    result[0] = size0;
    return result;
}

static inline std::array<int, 2> make_array (int size0, int size1)
{
    std::array<int, 2> result;
    result[0] = size0;
    result[1] = size1;
    return result;
}

static inline std::array<int, 3> make_array (int size0, int size1, int size2)
{
    std::array<int, 3> result;
    result[0] = size0;
    result[1] = size1;
    result[2] = size2;
    return result;
}

template <size_t N>
static inline std::array<int, N> make_stride (const std::array<int, N> &size)
{
    std::array<int, N> stride;

    stride[N - 1] = 1;
    for (int i = (int)N - 2; i >= 0; i--)
    {
        stride[i] = stride[i + 1] * size[i + 1];
    }

    return stride;
}

/// Class to represent NDArrays, row major order, continuous memory
template <typename T, size_t Dim>
class BrainFlowArray
{

private:
    int length;
    std::array<int, Dim> size;
    std::array<int, Dim> stride;
    T *origin;

public:
    friend std::ostream &operator<<<> (std::ostream &out, const BrainFlowArray<T, Dim> &array);

    BrainFlowArray ()
        : length (0)
        , size (zero_array<Dim> ())
        , stride (make_stride (zero_array<Dim> ()))
        , origin (nullptr)
    {
    }

    ~BrainFlowArray ()
    {
        if (origin != nullptr)
        {
            delete[] origin;
        }
    }

    BrainFlowArray (const std::array<int, Dim> &size)
        : length (product (size)), size (size), stride (make_stride (size)), origin (nullptr)
    {
        origin = new T[product (size)];
        memset (origin, 0, sizeof (T) * product (size));
    }

    explicit BrainFlowArray (int size0)
        : length (size0)
        , size (make_array (size0))
        , stride (make_stride<1> (make_array (size0)))
        , origin (nullptr)
    {
        static_assert (Dim == 1, "This function is only for BrainFlowArray<T, 1>");
        origin = new T[size0];
        memset (origin, 0, sizeof (T) * size0);
    }

    BrainFlowArray (int size0, int size1)
        : length (size0 * size1)
        , size (make_array (size0, size1))
        , stride (make_stride<2> (make_array (size0, size1)))
        , origin (nullptr)
    {
        static_assert (Dim == 2, "This function is only for BrainFlowArray<T, 2>");
        origin = new T[length];
        memset (origin, 0, sizeof (T) * length);
    }

    BrainFlowArray (int size0, int size1, int size2)
        : length (size0 * size1 * size2)
        , size (make_array (size0, size1, size2))
        , stride (make_stride<3> (make_array (size0, size1, size2)))
        , origin (nullptr)
    {
        static_assert (Dim == 3, "This function is only for BrainFlowArray<T, 3>");
        origin = new T[length];
        memset (origin, 0, sizeof (T) * length);
    }

    BrainFlowArray (T *ptr, int size0)
        : length (size0)
        , size (make_array (size0))
        , stride (make_stride<1> (make_array (size0)))
        , origin (nullptr)
    {
        static_assert (Dim == 1, "This function is only for BrainFlowArray<T, 1>");
        origin = new T[size0];
        memcpy (origin, ptr, size0 * sizeof (T));
    }

    BrainFlowArray (T *ptr, int size0, int size1)
        : length (size0 * size1)
        , size (make_array (size0, size1))
        , stride (make_stride<2> (make_array (size0, size1)))
        , origin (nullptr)
    {
        static_assert (Dim == 2, "This function is only for BrainFlowArray<T, 2>");
        origin = new T[size0 * size1];
        memcpy (origin, ptr, (long)size0 * size1 * sizeof (T));
    }

    BrainFlowArray (T *ptr, int size0, int size1, int size2)
        : length (size0 * size1 * size2)
        , size (make_array (size0, size1, size2))
        , stride (make_stride<3> (make_array (size0, size1, size2)))
        , origin (nullptr)
    {
        static_assert (Dim == 3, "This function is only for BrainFlowArray<T, 3>");
        origin = new T[size0 * size1 * size2];
        memcpy (origin, ptr, size0 * size1 * size2 * sizeof (T));
    }

    BrainFlowArray (const BrainFlowArray &other)
        : length (other.length), size (other.size), stride (other.stride), origin (nullptr)
    {
        origin = new T[other.length];
        memcpy (origin, other.origin, sizeof (T) * other.length);
    }

    const BrainFlowArray &operator= (const BrainFlowArray &other)
    {
        length = other.length;
        size = other.size;
        stride = other.stride;
        origin = new T[other.length];
        memcpy (origin, other.origin, sizeof (T) * other.length);

        return *this;
    }

    BrainFlowArray (BrainFlowArray &&other)
        : length (other.length)
        , size (std::move (other.size))
        , stride (std::move (other.stride))
        , origin (nullptr)
    {
        origin = other.origin;
        other.origin = nullptr;
        other.length = 0;
        other.size = zero_array<Dim> ();
    }

    const BrainFlowArray &operator= (BrainFlowArray &&other)
    {
        length = other.length;
        size = std::move (other.size);
        stride = std::move (other.stride);
        if (origin != nullptr)
        {
            delete[] origin;
        }
        origin = other.origin;
        other.origin = nullptr;
        other.length = 0;
        other.size = zero_array<Dim> ();
        return *this;
    }

    /// get size of data type
    int get_item_size () const
    {
        return sizeof (T);
    }

    /// get total length
    int get_length () const
    {
        return length;
    }

    /// get ndims
    int get_ndims () const
    {
        return Dim;
    }

    /// get dims size
    const std::array<int, Dim> &get_sizes () const
    {
        return size;
    }

    /// get size in dim
    int get_size (int dim) const
    {
        if ((dim < 0) || (dim >= Dim))
        {
            throw BrainFlowException (
                "invalid dim argument", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
        }
        return size[dim];
    }

    const std::array<int, Dim> &get_strides () const
    {
        return stride;
    }

    int get_stride (int dim) const
    {
        return stride[dim];
    }

    /// get raw pointer
    T *get_raw_ptr ()
    {
        return origin;
    }

    /// get raw pointer
    const T *get_raw_ptr () const
    {
        return origin;
    }

    operator T * ()
    {
        return get_raw_ptr ();
    }

    operator const T * () const
    {
        return get_raw_ptr ();
    }

    /// access element at index
    T &at (int index0)
    {
        int idx = index0 * get_stride (0);
        if (idx >= length)
        {
            throw BrainFlowException (
                "out of range", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
        }
        return origin[idx];
    }

    /// access element at index
    const T &at (int index0) const
    {
        int idx = index0 * get_stride (0);
        if (idx >= length)
        {
            throw BrainFlowException (
                "out of range", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
        }
        return origin[idx];
    }

    /// access element at index
    T &operator() (int index0)
    {
        return at (index0);
    }

    /// access element at index
    const T &operator() (int index0) const
    {
        return at (index0);
    }

    /// access element at index
    T &operator[] (int index0)
    {
        return origin[index0];
    }

    /// access element at index
    const T &operator[] (int index0) const
    {
        return origin[index0];
    }

    /// access element at index
    T &at (int index0, int index1)
    {
        static_assert (Dim >= 2, "BrainFlowArray dimension bounds error");
        int idx = index0 * get_stride (0) + index1 * get_stride (1);
        if (idx >= length)
        {
            throw BrainFlowException (
                "out of range", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
        }
        return origin[idx];
    }

    /// access element at index
    const T &at (int index0, int index1) const
    {
        static_assert (Dim >= 2, "BrainFlowArray dimension bounds error");
        int idx = index0 * get_stride (0) + index1 * get_stride (1);
        if (idx >= length)
        {
            throw BrainFlowException (
                "out of range", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
        }
        return origin[idx];
    }

    /// access element at index
    T &operator() (int index0, int index1)
    {
        return at (index0, index1);
    }

    /// access element at index
    const T &operator() (int index0, int index1) const
    {
        return at (index0, index1);
    }

    /// access element at index
    T &at (int index0, int index1, int index2)
    {
        static_assert (Dim >= 3, "BrainFlowArray dimension bounds error");
        int idx = index0 * get_stride (0) + index1 * get_stride (1) + index2 * get_stride (2);
        if (idx >= length)
        {
            throw BrainFlowException (
                "out of range", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
        }
        return origin[idx];
    }

    /// access element at index
    const T &at (int index0, int index1, int index2) const
    {
        static_assert (Dim >= 3, "BrainFlowArray dimension bounds error");
        int idx = index0 * get_stride (0) + index1 * get_stride (1) + index2 * get_stride (2);
        if (idx >= length)
        {
            throw BrainFlowException (
                "out of range", (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR);
        }
        return origin[idx];
    }

    /// access element at index
    T &operator() (int index0, int index1, int index2)
    {
        return at (index0, index1, index2);
    }

    /// access element at index
    const T &operator() (int index0, int index1, int index2) const
    {
        return at (index0, index1, index2);
    }

    /// check if array is empty
    bool empty () const
    {
        return get_raw_ptr () == nullptr || get_length () == 0;
    }

    /// use it to get pointer to row in matrix or to get pointer to matrix from 3d array
    T *get_address (int index0)
    {
        return &origin[index0 * get_stride (0)];
    }

    /// use it to get pointer to row in 3d array
    T *get_address (int index0, int index1)
    {
        static_assert (Dim >= 2, "Dim should be >= 2");
        return &origin[index0 * get_stride (0) + index1 * get_stride (1)];
    }

    /// use it to get pointer to row in matrix or to get pointer to matrix from 3d array
    const T *get_address (int index0) const
    {
        return &origin[index0 * get_stride (0)];
    }

    /// use it to get pointer to row in 3d array
    const T *get_address (int index0, int index1) const
    {
        static_assert (Dim >= 2, "Dim should be >= 2");
        return &origin[index0 * get_stride (0) + index1 * get_stride (1)];
    }

    /// fill already preallocated buffer
    void fill (T *ptr, int size)
    {
        memcpy (origin, ptr, sizeof (T) * size);
    }
};

template <typename T, size_t Dim>
std::ostream &operator<< (std::ostream &os, const BrainFlowArray<T, Dim> &array)
{
    os << "Strides:" << std::endl;
    std::array<int, Dim> strides = array.get_strides ();
    for (int i = 0; i < Dim; i++)
    {
        os << strides[i] << " ";
    }
    os << std::endl << "Sizes:" << std::endl;
    std::array<int, Dim> sizes = array.get_sizes ();
    for (int i = 0; i < Dim; i++)
    {
        os << sizes[i] << " ";
    }
    os << std::endl;

    const T *raw_ptr = array.get_raw_ptr ();
    if (Dim == 2)
    {
        for (int i = 0; i < sizes[0]; i++)
        {
            for (int j = 0; j < std::min<int> (sizes[1], 10); j++)
            {
                int idx = i * strides[0] + j * strides[1];
                os << std::to_string (raw_ptr[idx]) << " ";
            }
            os << std::endl;
        }
        os << std::endl;
    }
    else
    {
        int len = array.get_length ();
        for (int i = 0; i < std::min<int> (len, 10); i++)
        {
            os << std::to_string (raw_ptr[i]) << " ";
        }
        os << std::endl;
    }
    return os;
}
