#pragma once

#include "Storage.h"
#include "FormatContextWrapper.h"

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
		static Dictionary<String^, String^>^ GetMetaData(String^ file);

		FFmpeg(Stream^ streamIn, Stream^ streamOut);
		FFmpeg(Stream^ streamIn, String^ fileOut);

		FFmpeg(String^ fileIn, String^ fileOut);
		FFmpeg(String^ fileIn, Stream^ streamOut);
		void DoStuff();

	private:
		static Dictionary<String^, String^>^ GetMetaData_(FormatContextWrapper% file);
	
		~FFmpeg();
		!FFmpeg();

		// Hack to not have interior_ptr<T> but native pointers
		Storage* storage;

		FormatContextWrapper^ dataIn;
		FormatContextWrapper^ dataOut;

		int streamIndex;

		inline void InitInput_();
		inline void InitOutput_();
		inline void InitFilter_();

		inline void DoStuff_();

		inline void DecodePacket_(AVPacket* pPacket, AVFrame* pFrame, AVFrame* pFilterFrame);
		inline void FilterFrame_(AVFrame* pFrame, AVFrame* pFilterFrame);
		inline void EncodeWriteFrame_(AVFrame* pFilterFrame);
	};
}
