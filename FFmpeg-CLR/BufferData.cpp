#include "BufferData.h"

#include <stdint.h>

#include <algorithm>
#include <iterator>

#include <libavformat/avformat.h>

namespace FFmpeg
{
	BufferData::BufferData(unsigned char* buffer, long long length) : buffer(buffer), length(length), position(0) {}

	int BufferData::ReadFunc(void* opaque, unsigned char* buf, int bufSize)
	{
		BufferData* pData = static_cast<BufferData*>(opaque);

		long long rest = pData->length - pData->position - 1;
		long long read = bufSize;
		if (bufSize > rest) read = rest;

		std::copy(
			pData->buffer + pData->position,
			pData->buffer + pData->position + read,
			buf
		);

		return read == 0 ? AVERROR_EOF : static_cast<int>(read);
	}

	int BufferData::WriteFunc(void* opaque, unsigned char* buf, int bufSize)
	{
		return 0;
	}

	long long BufferData::SeekFunc(void* opaque, long long offset, int whence)
	{
		BufferData* pData = static_cast<BufferData*>(opaque);
		
		if ((whence & AVSEEK_SIZE) == AVSEEK_SIZE) return pData->length;
		if (whence == SEEK_SET) pData->position = offset;
		else if (whence == SEEK_CUR) pData->position = pData->position + offset;
		else if (whence == SEEK_END) pData->position = pData->length + offset;

		return pData->position;
	}
}
