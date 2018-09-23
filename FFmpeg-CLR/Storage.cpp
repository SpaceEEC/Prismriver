#include "Storage.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
}

namespace FFmpeg
{
	Storage::~Storage()
	{
		if (this->inputFormatContext != nullptr)
		{
			avformat_close_input(&this->inputFormatContext);
		}

		if (this->outputFormatContext != nullptr)
		{
			avformat_close_input(&this->outputFormatContext);
		}

		if (this->inputIOContext != nullptr)
		{
			av_freep(this->inputIOContext);
		}

		if (this->encoderContext != nullptr)
		{
			avcodec_close(this->encoderContext);
			avcodec_free_context(&this->encoderContext);
		}
		if (this->decoderContext != nullptr)
		{
			avcodec_close(this->encoderContext);
			avcodec_free_context(&this->decoderContext);
		}
		// Do not "free" codecs
		this->decoder = nullptr;
		this->encoder = nullptr;

		avfilter_free(this->bufferSourceContext);
		avfilter_free(this->bufferSinkContext);

		avfilter_graph_free(&this->filterGraph);
	}
}
