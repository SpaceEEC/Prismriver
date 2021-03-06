#pragma once

#include "AVException.h"
#include "FormatContextWrapper.h"

#include <winerror.h>

namespace Prismriver
{
	private class CodecContextWrapper : public FormatContextWrapper
	{
	private:
		// No default or copy constructor or assign operator
		CodecContextWrapper() = delete;
		CodecContextWrapper(const CodecContextWrapper& other) = delete;
		CodecContextWrapper& operator=(const CodecContextWrapper& other) = delete;

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
		 * Returns the stream which is being read from.
		 * Will return nullptr if the CodecContextWrapper was opened via openRead or no stream was found.
		 */
		AVStream* GetStream();

		/**
		 * Opens this CodecContextWrapper in reading mode.
		 * Throw on failure.
		 */
		void OpenRead();

		/**
		 * Opens this CodecContextWrapper in writing mode.
		 * Throws on failure.
		 */
		void OpenWrite(CodecContextWrapper* dataIn);
	};
}