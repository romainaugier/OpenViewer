#include "OpenImageIO/imagecache.h"
#include "GL/gl3w.h"

#include <thread>
#include <string>
#include <vector>
#include <stdlib.h>
#include <filesystem>

#include "utils/string_utils.h"

enum FileType_
{
	FileType_Exr   = 0x1,
	FileType_Png   = 0x2,
	FileType_Jpg   = 0x4,
	FileType_Mov   = 0x8,
	FileType_Mp4   = 0x10,
	FileType_Other = 0x20
};

enum Format_
{
	Format_Float = 0x1 | GL_FLOAT,
	Format_U32   = 0x2 | GL_UNSIGNED_INT,
	Format_U8    = 0x4 | GL_UNSIGNED_BYTE
};

struct Image
{
	std::string path;
	uint64_t size;
	uint32_t xres;
	uint32_t yres;
	uint32_t channels;
	int16_t cache_index = -1;
	FileType_ type;
	Format_ format;

	Image() {}

	Image(std::string& fp)
	{
		path = fp;

		if(endsWith(fp, ".exr"))
		{
			type = FileType_Exr;
			format = Format_Float;
		}
		else if(endsWith(fp, ".png"))
		{
			type = FileType_Png;
			format = Format_U8;
		}
		else if(endsWith(fp, "jpg") || endsWith(fp, "jpeg"))
		{
			type = FileType_Jpg;
			format = Format_U8;
		}
		else
		{
			type = FileType_Other;
			format = Format_Float;
		}

		auto in = OIIO::ImageInput::open(fp);
		const OIIO::ImageSpec& spec = in->spec();

		xres = spec.width;
		yres = spec.height;
		channels = spec.nchannels;

		size = xres * yres * channels; // *sizeof(float);

		in->close();
	}

	void release();
	void load(void* allocated_space);
	void load_exr(float* allocated_space);
	void load_png(uint8_t* allocated_space);
	void load_jpg(uint8_t* allocated_space);
	void load_other(float* allocated_space);
};

struct Loader
{
	void* memory_arena;
	void* single_image;
	std::vector<Image> images;
	std::vector<std::thread> workers;
	std::vector<uint16_t> last_cached;
	std::vector<char> cached;
	mutable std::mutex mtx;
	uint64_t cache_size;
	uint64_t cached_size;
	uint64_t cache_stride;
	uint16_t count = 0;
	uint16_t frame = 0;
	uint16_t cache_size_count = 0;
	unsigned int is_playing : 1;
	unsigned int has_finished : 1;
	unsigned int is_working : 1;
	unsigned int is_playloader_working : 1;

	Loader() 
	{
		cached_size = 0;
		has_finished = 0;
		is_working = 0;
		is_playloader_working = 0;
	}

	~Loader()
	{
	}

	void initialize(std::string& fp, uint64_t _cache_size);
	void load_sequence();
	void load_player();
	void load_images(uint16_t idx, uint8_t number);
	void load_image(uint16_t idx);
	void unload_images(uint8_t number);
	void unload_image();
	void launch_sequence_worker();
	void launch_cacheload_worker(uint16_t idx, uint16_t number);
	void launch_player_worker();
	void join_worker();
	void release();
};

