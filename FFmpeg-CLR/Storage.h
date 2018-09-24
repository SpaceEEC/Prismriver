#pragma once

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/buffersink.h>
}

namespace FFmpeg
{
	private struct Storage
	{
	public:
		~Storage();

		AVFormatContext* inputFormatContext = nullptr;
		AVFormatContext* outputFormatContext = nullptr;

		AVIOContext* inputIOContext = nullptr;
		AVIOContext* outpuIOContext = nullptr;

		AVCodecContext* decoderContext = nullptr;
		AVCodecContext* encoderContext = nullptr;

		AVCodec* decoder = nullptr;
		AVCodec* encoder = nullptr;

		AVFilterContext* bufferSinkContext;
		AVFilterContext* bufferSourceContext;
		AVFilterGraph* filterGraph;
	};
}
