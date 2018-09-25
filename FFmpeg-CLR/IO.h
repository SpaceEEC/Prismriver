#pragma once

namespace FFmpeg
{
	namespace IO
	{
		int ReadFunc(void* opaque, unsigned char* buf, int buf_size);
		int WriteFunc(void* opaque, unsigned char* buf, int buf_size);
		long long SeekFunc(void* opaque, long long offset, int whence);
	}
}