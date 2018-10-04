#pragma once

#include "ITrack.h"

#include "FilterContextWrapper.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

using namespace System;
using namespace System::IO;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;

namespace Prismriver
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

		/**
		 * The ITracks to split the input.
		 */
		property array<ITrack^>^ Tracks
		{
			array<ITrack^>^ get() { return this->tracks_; }
		}

		/**
		 * Starts transcoding.
		 */
		void Run();
		/**
		 * Registers an IProgress to report progress to.
		 * First item of the tuple will be the index of the currently being transcoded track.
		 * Second item of the tuple will be the percent [0,1].
		 */
		Transcoder^ SetProgress(IProgress<Tuple<int, double>^>^ progress);

	private:
		~Transcoder();
		!Transcoder();

		/**
		 * IProgress to report progress to.
		 */
		IProgress<Tuple<int, double>^>^ progress_;

		/**
		 * Actually holds the ITracks to split the intput.
		 */
		array<ITrack^>^ tracks_ = nullptr;

		/**
		 * The index of the current track.
		 */
		int trackIndex_;

		/**
		 * Holds the relevant AV* classes for the input.
		 */
		CodecContextWrapper* dataIn_;

		/**
		 * Holds the relevant AV* classes for the current output.
		 * Only is valid while transcoding, will change for every track.
		 */
		FilterContextWrapper* dataOut_;

		/**
		 * Actually does the transcoding.
		 */
		inline void Run_();
		/**
		 * Initializes the metadata of the output format and streams.
		 */
		inline void InitMetaData_();

		/**
		 * Decodes a frame, this will call Transcoder#FilterFrame_ while doing so.
		 */
		inline void DecodePacket_(AVPacket* pPacket, AVFrame* pFrame, AVFrame* pFilterFrame, AVPacket* pEncodedPacket);
		/**
		 * Filters a frame, this will call Transcoder#EncodeWriteFrame_ while doing so.
		 */
		inline void FilterFrame_(AVFrame* pFrame, AVFrame* pFilterFrame, AVPacket* pEncodedPacket);
		/**
		 * Encodes and write a frame.
		 */
		inline void EncodeWriteFrame_(AVFrame* pFilterFrame, AVPacket* pEncodedPacket);

		/**
		 * Reports progress of the current timestamp, if set.
		 */
		void ReportProgress_(double current);
	};
}
