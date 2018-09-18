#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <winerror.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

namespace FFmpeg {
	public ref class FFmpeg sealed
	{
	public:
		FFmpeg(System::String^ fileIn, System::String^ fileOut);
		void DoStuff();

	private:
		static FFmpeg();
		~FFmpeg();
		!FFmpeg();

		char* m_FileIn;
		char* m_FileOut;
		int m_StreamIndex;
		int m_GotFrame;
		int m_FrameCount = 0;

		inline HRESULT Init_(AVFormatContext** ppFormatContext, AVCodec** ppCodec, AVCodecContext** ppCodecContext);
		inline HRESULT DoStuff_(AVFormatContext* pFormatContext, AVCodec* pCodec, AVCodecContext* pCodecContext);
		inline HRESULT DecodePacket_(std::ofstream* ifstream, AVCodecContext* pCodecContext, AVPacket* pPacket, AVFrame* pFrame);
	};
}
