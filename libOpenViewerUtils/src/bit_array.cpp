// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include "OpenViewerUtils/bit_array.h"
#include "OpenViewerUtils/memory.h"

LOVU_NAMESPACE_BEGIN

bit_array::bit_array()
{
    this->m_size = RESERVE_BITS(256);

    this->m_array = static_cast<uint64_t*>(mem_alloc(this->m_size * sizeof(BITARRAY_INT_TYPE), 32));

    memset(this->m_array, 0, this->get_byte_size());
}

bit_array::bit_array(const size_t& size)
{   
    this->m_size = RESERVE_BITS(size);
    
    this->m_array = static_cast<uint64_t*>(mem_alloc(this->m_size * sizeof(BITARRAY_INT_TYPE), 32));
    
    memset(this->m_array, 0, this->get_byte_size());
}

bit_array::~bit_array()
{
    mem_free(this->m_array);
}

void bit_array::set(const size_t& position) noexcept
{
    LOVU_ASSERT(position < get_size());

    this->m_array[position / SIZEOF_BITARRAY_INT] |= 1 << (position % SIZEOF_BITARRAY_INT);
}

void bit_array::clear(const size_t& position) noexcept
{
    LOVU_ASSERT(position < get_size());

    this->m_array[position / SIZEOF_BITARRAY_INT] &= ~(1 << (position % SIZEOF_BITARRAY_INT));
}

void bit_array::clear_all() noexcept
{
    memset(this->m_array, 0, this->get_byte_size());
}

bool bit_array::test(const size_t& position) const noexcept
{
    LOVU_ASSERT(position < get_size());

    return ((this->m_array[position / SIZEOF_BITARRAY_INT] & 1 << (position % SIZEOF_BITARRAY_INT)) != 0);
}

size_t bit_array::get_size() const noexcept
{
    return this->m_size;
}

size_t bit_array::get_byte_size() const noexcept
{
    return this->m_size * sizeof(BITARRAY_INT_TYPE);
}

void bit_array::resize(const size_t& new_size) noexcept
{
    const size_t alloc_size = RESERVE_BITS(new_size) * sizeof(BITARRAY_INT_TYPE);

    if(alloc_size < this->get_byte_size()) return;

    void* new_ptr = mem_alloc(alloc_size, 32);

    memcpy(new_ptr, this->m_array, this->get_byte_size());
    
    this->m_size = RESERVE_BITS(new_size);

    mem_free(this->m_array);

    this->m_array = static_cast<uint64_t*>(new_ptr);
}

LOVU_NAMESPACE_END
