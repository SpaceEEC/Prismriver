#pragma once

namespace FFmpeg
{
	namespace IO
	{
		int ReadFunc(void* opaque, unsigned char* buf, int bufSize);
		int WriteFunc(void* opaque, unsigned char* buf, int bufSize);
		long long SeekFunc(void* opaque, long long offset, int whence);
	}
}