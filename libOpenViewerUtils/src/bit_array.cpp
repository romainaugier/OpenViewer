// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewerUtils/bit_array.h"
#include "OpenViewerUtils/memory.h"

LOVU_NAMESPACE_BEGIN

bit_array::bit_array()
{
    m_array = static_cast<uint64_t*>(mem_alloc(RESERVE_BITS(64), 32));
}

bit_array::bit_array(const size_t& size)
{   
    m_array = static_cast<uint64_t*>(mem_alloc(RESERVE_BITS(size), 32));
}

bit_array::~bit_array()
{
    mem_free(m_array);
}

void bit_array::set(const size_t& position) noexcept
{
    LOVU_ASSERT(position < get_size());

    m_array[position / SIZEOF_BITARRAY_INT] |= 1 << (position % SIZEOF_BITARRAY_INT);
}

void bit_array::clear(const size_t& position) noexcept
{
    LOVU_ASSERT(position < get_size());

    m_array[position / SIZEOF_BITARRAY_INT] &= ~(1 << (position % SIZEOF_BITARRAY_INT));
}

bool bit_array::test(const size_t& position) const noexcept
{
    LOVU_ASSERT(position < get_size());

    return ((m_array[position / SIZEOF_BITARRAY_INT] & 1 << (position % SIZEOF_BITARRAY_INT)) != 0);
}

size_t bit_array::get_size() const noexcept
{
    return LOVUARRAYSIZE(m_array) * SIZEOF_BITARRAY_INT;
}

void bit_array::resize(const size_t& new_size) noexcept
{
    if(new_size <= get_size()) return;
    else
    {
        void* new_ptr = mem_alloc(RESERVE_BITS(new_size), 32);

        memcpy(new_ptr, m_array, new_size);

        mem_free(m_array);

        m_array = static_cast<uint64_t*>(new_ptr);
    }
}

LOVU_NAMESPACE_END