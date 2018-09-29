#pragma once

#include "AVException.h"
#include "FormatContextWrapper.h"

#include <winerror.h>

namespace FFmpeg
{
	private class CodecContextWrapper : public FormatContextWrapper
	{
	private:
		// No default or copy constructor or assign operator
		CodecContextWrapper() = delete;
		CodecContextWrapper(const CodecContextWrapper& other) = delete;
		CodecContextWrapper& operator=(const CodecContextWrapper% other) = delete;

	public:
		CodecContextWrapper(Stream^ stream) : FormatContextWrapper(stream) {}
		CodecContextWrapper(String^ file) : FormatContextWrapper(file) {}

		virtual ~CodecContextWrapper();
		
		/**
		 * The index of the found stream.
		 * Will only not be AVERROR_STREAM_NOT_FOUND if opened in read mode.
		 */
		int streamIndex = AVERROR_STREAM_NOT_FOUND;

		/**
		 * The AVCodec for this CodecContextWrapper.
		 * Will be nullptr if not opened via openRead or openWrite.
		 */
		AVCodec* codec = nullptr;

		/**
		 * The AVCodecContext for this CodecContextWrapper.
		 * Will be nullptr if not opened via openRead or openWrite.
		 */
		AVCodecContext* codecContext = nullptr;

		/**
		 * Opens this CodecContextWrapper in reading mode.
		 * Throw on failure.
		 */
		void openRead();

		// TODO: This should probably not explicitly require an input.
		/**
		 * Opens this CodecContextWrapper in writing mode.
		 * Throws on failure.
		 */
		void openWrite(CodecContextWrapper* dataIn);
	};
}