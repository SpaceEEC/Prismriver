#include "Storage.h"

namespace FFmpeg
{
	Storage::~Storage()
	{
		if (this->encoderContext != nullptr)
		{
			avcodec_free_context(&this->encoderContext);
		}
		if (this->decoderContext != nullptr)
		{
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
