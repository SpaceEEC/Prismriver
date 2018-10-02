#pragma once

extern "C"
{
#include <libavcodec/avcodec.h>
}

namespace Prismriver
{
	public enum class ImageFormat : int
	{
		JPEG = AV_CODEC_ID_MJPEG,
		PNG = AV_CODEC_ID_PNG,
		BMP = AV_CODEC_ID_BMP
	};
}
