// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "openviewerutils.h"

LOVU_NAMESPACE_BEGIN

// We use 64 bits integers in the bit array
#define SIZEOF_BITARRAY_INT 64 
#define BITARRAY_INT_TYPE uint64_t

// Calculate the number of uint64_t we need to hold how many bytes are requested
#define RESERVE_BITS(bits) (bits + 0x3F) / SIZEOF_BITARRAY_INT

// bit_array is a simple class that holds a bit field consisting of unsigned 64 bits integers
// and a few methods to set, clear, test and resize the array
class LOVU_API bit_array
{
public:
    // Default constructor with a size of 64 bits
    bit_array();

    // Constructs a bit_array
    bit_array(const size_t& size);

    // Destructs a bit_array
    ~bit_array();

    // Sets a bit at the given position. If the position is greater than the bit_array size, 
    // bit_array will grow to reach the position
    void set(const size_t& position) noexcept;

    // Clears a bit at the given position. If the position is greater than the bit_array size, 
    // bit_array will grow to reach the position
    void clear(const size_t& position) noexcept;

    // Tests a bit at the given position
    bool test(const size_t& position) const noexcept;

    // Returns the size of the array
    size_t get_size() const noexcept;
    
    // Resizes the array
    void resize(const size_t& new_size) noexcept;

private:
    uint64_t* m_array = nullptr;
    uint32_t m_size = 0;
};

LOVU_NAMESPACE_END