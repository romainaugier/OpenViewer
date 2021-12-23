// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Romain Augier
// All rights reserved.

#include "loader.h"

namespace Core
{
	Loader::Loader(Logger* logger, Profiler* profiler)
	{
		this->m_Logger = logger;
		this->m_Profiler = profiler;

		this->m_Cache = new ImageCache;
	}

	void Loader::Initialize(const bool useCache, const size_t cacheSize) noexcept
	{
		this->m_Logger->Log(LogLevel_Debug, "[LOADER] : Initializing loader");

		this->m_HasBeenInitialized = true;

		// Allocate the cache (if used)
		if (useCache)
		{
			this->m_UseCache = true;
			this->m_Cache->Initialize(cacheSize, this->m_Logger);
		}
	}

	void Loader::Load(const std::string& mediaPath, const bool directory) noexcept
	{

	}

	void Loader::LoadImageToCache(const uint16_t index) noexcept
	{
		if(this->m_UseCache)
		{
			const uint16_t cachedIndex = this->m_Cache->Add(&this->m_Images[index]);

			this->m_Images[index].Load(this->m_Cache->m_Items[cachedIndex].m_Ptr, this->m_Profiler);
		}
		else
		{
			this->m_Images[index].Load(this->m_Cache->m_Items[1].m_Ptr, this->m_Profiler);
		}
	}

	void Loader::LoadSequenceToCache(const uint16_t startIndex, const uint16_t size = 0) noexcept
	{
		
	}

	void Loader::Release() noexcept
	{
		// Release cache
		this->m_Cache->Release();
		delete this->m_Cache;

		// Clear the medias
		for(auto& media : this->m_Medias) 
		{
			media.m_Images.clear();
		}

		// Clear all the vectors
		this->m_Medias.clear();
		this->m_Medias.resize(0);

		this->m_Workers.clear();
		this->m_Workers.resize(0);
	}
}