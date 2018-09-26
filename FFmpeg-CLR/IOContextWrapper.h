#pragma once

extern "C"
{
#include <libavutil/mem.h>
#include <libavformat/avformat.h>
}

using namespace System;
using namespace System::IO;
using namespace System::Runtime::InteropServices; 

namespace FFmpeg
{
	static const int BUFFERSIZE = 16 * 1024;

	private ref struct IOContextWrapper
	{
	private:
		// No copy constructor or assign operator
		IOContextWrapper(const IOContextWrapper% other) { throw gcnew NotSupportedException(); }
		IOContextWrapper% operator=(const IOContextWrapper% other) { throw gcnew NotSupportedException(); }

		/**
		 * Whether this IOContextWrapper had already been opened.
		 */
		bool opened_ = false;

		/**
		 * The GCHandle of the wrapped stream.
		 */
		GCHandle handle_;

		/**
		 * Opens the AVIOContext in the passed mode.
		 */
		void open(bool write);
	public:
		/**
		 * The wrapped AVIOContext, will be nullptr if not opened yet.
		 */
		AVIOContext* ioContext = nullptr;

		/**
		 * Instantiates a new IOContextWrapper.
		 */
		IOContextWrapper(Stream^ stream) : handle_(GCHandle::Alloc(stream)) {}

		~IOContextWrapper() { this->!IOContextWrapper(); }
		!IOContextWrapper();

		/**
		 * Opens this IOContextWrapper in read mode.
		 * Throws on failure.
		 */
		void openRead() { this->open(true); }

		/**
		 * Opens this IOContextWrapper in write mode.
		 * Throws on failure.
		 */
		void openWrite() { this->open(false); }
	};
}