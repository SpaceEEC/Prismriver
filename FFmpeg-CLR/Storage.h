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

		AVFilterContext* bufferSinkContext;
		AVFilterContext* bufferSourceContext;
		AVFilterGraph* filterGraph;
	};
}
