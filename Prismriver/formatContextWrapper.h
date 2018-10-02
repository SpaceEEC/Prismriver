#pragma once

#include "IOContextWrapper.h"
#include "TrackTarget.h"
#include "Utils.h"

using namespace System;
using namespace System::IO;

extern "C"
{
#include <libavformat/avformat.h>
}

namespace Prismriver
{
	private class FormatContextWrapper
	{
	protected:
		// No default or copy constructor or assign operator
		FormatContextWrapper() = delete;
		FormatContextWrapper(const FormatContextWrapper& other) = delete;
		FormatContextWrapper& operator=(const FormatContextWrapper% other) = delete;

		/**
		 * The IOContextWrapper of this FormatContextWrapper, or nullptr if none.
		 */
		IOContextWrapper* ioContextWrapper_ = nullptr;

		/**
		 * The file path of the input file, or nullptr if none.
		 */
		char* file_ = nullptr;

		/**
		 * Used to overwrite the output format
		 */
		char* format_ = nullptr;

		/**
		 * Whether this FormatContextWrapper had already been opened.
		 */
		bool opened_ = false;

		/**
		 * Whether this FormatContextWrapper wraps an input context.
		 */
		bool input_ = false;

	public:

		/**
		 * Instantiates a new FormatContextWrapper wrapping a TrackTarget.
		 */
		FormatContextWrapper(TrackTarget^ target) :
			file_(target->File == nullptr ? nullptr : Utils::StringToUtf8Bytes(target->File)),
			ioContextWrapper_(target->Stream == nullptr ? nullptr : new IOContextWrapper(target->Stream)),
			format_(target->Format == nullptr ? nullptr : Utils::StringToUtf8Bytes(target->Format)) {}

		/**
		 * Instantiates a new FormatContextWrapper wrapping a Stream.
		 */
		FormatContextWrapper(Stream^ stream) : ioContextWrapper_(new IOContextWrapper(stream)) {}
		/**
		 * Instantiates a new FormatContextWrapper using a file path.
		 */
		FormatContextWrapper(String^ file) : file_(Utils::StringToUtf8Bytes(file)) {}

		virtual ~FormatContextWrapper();

		/**
		 * The AVFormatContext of this FormatContextWrapper
		 * Will be nullptr if not opened via openRead or openWrite.
		 */
		AVFormatContext* formatContext = nullptr;

		/**
		 * Opens this FormatContextWrapper in reading mode.
		 * Throws on failure.
		 */
		virtual void openRead();
		/**
		 * Opens this FormatContextWrapper in writing mode.
		 * Throws on failure.
		 */
		virtual void openWrite();
	};
}

