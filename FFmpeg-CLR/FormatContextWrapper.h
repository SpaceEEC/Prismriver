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
	private ref class FormatContextWrapper
	{
	private:
		// No copy constructor or assign operator
		FormatContextWrapper(const FormatContextWrapper% other) { throw gcnew NotSupportedException(); }
		FormatContextWrapper% operator=(const FormatContextWrapper% other) { throw gcnew NotSupportedException(); }

		/**
		 * The IOContextWrapper of this FormatContextWrapper, or nullptr if none.
		 */
		IOContextWrapper^ ioContextWrapper_ = nullptr;

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

	internal:
		/**
		 * Overrides the output format
		 */
		void setOutFormat(String^ format) { this->format_ = static_cast<const char*>(Marshal::StringToHGlobalAnsi(format).ToPointer()); }

	public:
		/**
		 * Instantiates a new FormatContextWrapper wrapping a Stream.
		 */
		FormatContextWrapper(Stream^ stream) : ioContextWrapper_(gcnew IOContextWrapper(stream)) {}
		/**
		 * Instantiates a new FormatContextWrapper using a file path.
		 */
		FormatContextWrapper(String^ file) : file_(static_cast<const char*>(Marshal::StringToHGlobalAnsi(file).ToPointer())) {}

		~FormatContextWrapper() { this->!FormatContextWrapper(); }
		!FormatContextWrapper();

		/**
		 * The AVFormatContext of this FormatContextWrapper
		 * Will be nullptr if not opened via openRead or openWrite.
		 */
		AVFormatContext* formatContext = nullptr;

		/**
		 * Opens this FormatContextWrapper in reading mode.
		 * Throws on failure.
		 */
		void openRead();
		/**
		 * Opens this FormatContextWrapper in writing mode.
		 * Throws on failure.
		 */
		void openWrite();
	};
}

