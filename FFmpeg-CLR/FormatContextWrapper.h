#pragma once

using namespace System;
using namespace System::IO;

#include <winerror.h>

extern "C"
{
#include <libavformat/avformat.h>
}

#include "IOContextWrapper.h"

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
		 * The file path of the input file, or NULL if none.
		 */
		const char* file_ = nullptr;

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
		 * Returns HRESULT of whether it was a success.
		 */
		HRESULT openRead();
		/**
		 * Opens this FormatContextWrapper in writing mode.
		 * Returns HRESULT of whether it was a success.
		 */
		HRESULT openWrite();
	};
}

