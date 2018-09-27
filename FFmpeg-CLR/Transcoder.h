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
	public ref class Transcoder sealed
	{
	public:
		/**
		 * Instantiates a new Transcoder.
		 */
		Transcoder();

		/**
		 * Sets the input for this FFmpeg to a stream.
		 */
		Transcoder^ SetIn(Stream^ stream);
		/**
		 * Sets the input for this FFmpeg to a file.
		 */
		Transcoder^ SetIn(String^ file);

		/**
		 * Sets the output for this FFmpeg to a stream.
		 * Note that setting an output format with FFmpeg#SetOutFormat is required.
		 */
		Transcoder^ SetOut(Stream^ file);
		/**
		 * Sets the output for this FFmpge to a file.
		 * Note that the output format will be inferred from the provided file if not overriden with FFmpeg#SetOutFormat.
		 */
		Transcoder^ SetOut(String^ file);

		/**
		 * Sets or overrides the output format for this FFmpeg.
		 */
		Transcoder^ SetOutFormat(String^ format);

		void DoStuff();

	private:
		~Transcoder();
		!Transcoder();

		/**
		 * Temporary holder for a set output format.
		 */
		String^ format_;

		/**
		 * Hack to not have interior_ptr<T> but native pointers.
		 */
		Storage* storage;

		/**
		 * Holds the relevant AV* classes for the input.
		 */
		FormatContextWrapper* dataIn;
		/**
		 * Holds the relevant AV* classes for the output.
		 */
		FormatContextWrapper* dataOut;

		/**
		 * The index to the audio stream being read from.
		 */
		int streamIndex;

		/**
		 * Initializes the input.
		 */
		inline void InitInput_();
		/**
		 * Initializes the output.
		 */
		inline void InitOutput_();
		/**
		 * Initializes the filter. (buffersink / buffersource)
		 */
		inline void InitFilter_();

		inline void DoStuff_();

		/**
		 * Decodes a frame, this will call Transcoder#FilterFrame_ while doing so.
		 */
		inline void DecodePacket_(AVPacket* pPacket, AVFrame* pFrame, AVFrame* pFilterFrame);
		/**
		 * Filters a frame, this will call Transcoder#EncodeWriteFrame_ while doing so.
		 */
		inline void FilterFrame_(AVFrame* pFrame, AVFrame* pFilterFrame);
		/**
		 * Encodes and write a frame.
		 */
		inline void EncodeWriteFrame_(AVFrame* pFilterFrame);
	};
}
