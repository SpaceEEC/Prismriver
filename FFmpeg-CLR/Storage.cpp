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
		// Do not "free" a codec
		this->m_InputCodec = nullptr;

		if (this->m_InputFormatContext != nullptr)
		{
			avformat_close_input(&this->m_InputFormatContext);
			avformat_free_context(this->m_InputFormatContext);
		}

		if (this->m_InputCodecContext != nullptr)
			avcodec_free_context(&this->m_InputCodecContext);
	}
}
