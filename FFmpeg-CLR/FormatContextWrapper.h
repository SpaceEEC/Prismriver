#pragma once

#include "IOContextWrapper.h"

using namespace System;
using namespace System::IO;

extern "C"
{
#include <libavformat/avformat.h>
}

namespace FFmpeg
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
		const char* file_ = nullptr;

		/**
		 * Used to overwrite the output format
		 */
		const char* format_ = nullptr;

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
		 * Instantiates a new FormatContextWrapper wrapping a Stream.
		 */
		FormatContextWrapper(Stream^ stream) : ioContextWrapper_(new IOContextWrapper(stream)) {}
		/**
		 * Instantiates a new FormatContextWrapper using a file path.
		 */
		FormatContextWrapper(String^ file) : file_(static_cast<const char*>(Marshal::StringToHGlobalAnsi(file).ToPointer())) {}

		virtual ~FormatContextWrapper();

		/**
		 * The AVFormatContext of this FormatContextWrapper
		 * Will be nullptr if not opened via openRead or openWrite.
		 */
		AVFormatContext* formatContext = nullptr;

		/**
		 * Overrides the output format
		 */
		void setOutFormat(String^ format) { if (this->format_ == nullptr) this->format_ = static_cast<const char*>(Marshal::StringToHGlobalAnsi(format).ToPointer()); }

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

