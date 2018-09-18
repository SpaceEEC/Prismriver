#pragma once

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

namespace FFmpeg
{
	private struct Storage
	{
	public:
		AVFormatContext* m_InputFormatContext = nullptr;
		AVCodec* m_InputCodec = nullptr;
		AVCodecContext* m_InputCodecContext = nullptr;
		~Storage();
	};
}
