#pragma once

#include <winerror.h>

#include "Storage.h"
#include "BufferData.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}



using namespace System;
using namespace System::IO;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;

namespace FFmpeg
{
	public ref class FFmpeg sealed
	{
	public:
		static Dictionary<String^, String^>^ GetMetaData(Stream^ stream);

		FFmpeg(Stream^ streamIn, Stream^ streamOut);
		void DoStuff();

	private:
		~FFmpeg();
		!FFmpeg();

		// Hack to not have interior_ptr<T> but native pointers
		Storage* storage;

		Buffer::Data^ dataIn;
		Buffer::Data^ dataOut;

		int streamIndex;

		inline HRESULT InitInput_();
		inline HRESULT InitOutput_();
		inline HRESULT InitFilter_();

		inline HRESULT DoStuff_();

		inline void DecodePacket_(AVPacket* pPacket, AVFrame* pFrame, AVFrame* pFilterFrame);
		inline void FilterFrame_(AVFrame* pFrame, AVFrame* pFilterFrame);
		inline void EncodeWriteFrame_(AVFrame* pFilterFrame);
	};
}
