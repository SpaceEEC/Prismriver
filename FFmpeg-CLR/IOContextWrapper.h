#pragma once

#include <winerror.h>

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
		 * Whether this IOContextWrapper wraps an output stream.
		 */
		bool write_ = false;

		/**
		 * The GCHandle of the wrapped stream.
		 */
		GCHandle handle_;

		/**
		 * Opens the AVIOContext in the passed mode.
		 */
		HRESULT open(bool write);
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
		 */
		HRESULT openRead() { return this->open(false); }

		/**
		 * Opens this IOContextWrapper in write mode.
		 */
		HRESULT openWrite() { return this->open(true); }
	};

	namespace Buffer
	{
		int ReadFunc(void* opaque, unsigned char* buf, int buf_size);
		int WriteFunc(void* opaque, unsigned char* buf, int buf_size);
		long long SeekFunc(void* opaque, long long offset, int whence);
	}
}