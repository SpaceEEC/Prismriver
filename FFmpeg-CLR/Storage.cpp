#include "Storage.h"

namespace FFmpeg
{
	Storage::~Storage()
	{
		avfilter_free(this->bufferSourceContext);
		avfilter_free(this->bufferSinkContext);

		avfilter_graph_free(&this->filterGraph);
	}
}
