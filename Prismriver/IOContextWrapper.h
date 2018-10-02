#pragma once

extern "C"
{
#include <libavutil/mem.h>
#include <libavformat/avformat.h>
}

using namespace System;
using namespace System::IO;
using namespace System::Runtime::InteropServices;

namespace Prismriver
{
	static const int BUFFERSIZE = 16 * 1024;

	private struct IOContextWrapper
	{
	private:
		// No default or copy constructor or assign operator
		IOContextWrapper() = delete;
		IOContextWrapper(const IOContextWrapper& other) = delete;
		IOContextWrapper& operator=(const IOContextWrapper& other) = delete;

		/**
		 * Whether this IOContextWrapper had already been opened.
		 */
		bool opened_ = false;

		/**
		 * Pointer to the GCHandle of the wrapped stream.
		 */
		void* handle_;

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
		IOContextWrapper(Stream^ stream) : handle_(static_cast<void*>(static_cast<IntPtr>(GCHandle::Alloc(stream)))) { }

		~IOContextWrapper();

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