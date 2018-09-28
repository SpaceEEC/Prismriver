#pragma once

#include "ITrack.h"

#include "Storage.h"
#include "CodecContextWrapper.h"

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
		 * Instantiates a new Transcoder using a stream.
		 */
		Transcoder(Stream^ stream, array<ITrack^>^ tracks);
		/**
		 * Instantiates a new Transcoder using a file.
		 */
		Transcoder(String^ file, array<ITrack^>^ tracks);

		void Run();

	private:
		~Transcoder();
		!Transcoder();

		array<ITrack^>^ tracks;

		/**
		 * Hack to not have interior_ptr<T> but native pointers.
		 */
		Storage* storage;

		/**
		 * Holds the relevant AV* classes for the input.
		 */
		CodecContextWrapper* dataIn;

		/**
		 * The index to the audio stream being read from.
		 */
		int streamIndex;

		/**
		 * Initializes the filter. (buffersink / buffersource)
		 */
		inline void InitFilter_(CodecContextWrapper& out);

		inline void Run_(CodecContextWrapper& out, ITrack^ track);

		/**
		 * Decodes a frame, this will call Transcoder#FilterFrame_ while doing so.
		 */
		inline void DecodePacket_(CodecContextWrapper& out, AVPacket* pPacket, AVFrame* pFrame, AVFrame* pFilterFrame, AVPacket* pEncodedPacket);
		/**
		 * Filters a frame, this will call Transcoder#EncodeWriteFrame_ while doing so.
		 */
		inline void FilterFrame_(CodecContextWrapper& out, AVFrame* pFrame, AVFrame* pFilterFrame, AVPacket* pEncodedPacket);
		/**
		 * Encodes and write a frame.
		 */
		inline void EncodeWriteFrame_(CodecContextWrapper& out, AVFrame* pFilterFrame, AVPacket* pEncodedPacket);
	};
}
