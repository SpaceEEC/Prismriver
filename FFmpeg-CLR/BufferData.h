#pragma once

namespace FFmpeg
{
	private struct BufferData
	{
		unsigned char* buffer;
		long long position;
		long long length;

		BufferData(unsigned char* buffer, long long length);

		static int ReadFunc(void* opaque, unsigned char *buf, int buf_size);
		static int WriteFunc(void* opaque, unsigned char *buf, int buf_size);
		static long long SeekFunc(void* opaque, long long offset, int whence);
	};
}