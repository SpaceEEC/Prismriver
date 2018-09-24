#pragma once

using namespace System;
using namespace System::IO;
using namespace System::Runtime::InteropServices;

extern "C"
{
#include <libavutil/mem.h>
}

namespace FFmpeg
{
	namespace Buffer
	{
		static const int BUFFERSIZE = 16 * 1024;

		private ref struct Data
		{
		private:
			GCHandle handle;
			Data(const Data^ that)
			{
				throw gcnew Exception("nope");
			}

			Data^ operator=(const Data^ that)
			{
				throw gcnew Exception("nope");
			}

		public:
			void* stream;
			unsigned char* buffer;

			Data(Stream^ stream) : handle(GCHandle::Alloc(stream))
			{
				this->stream = GCHandle::ToIntPtr(this->handle).ToPointer();
				this->buffer = static_cast<unsigned char*>(av_malloc(BUFFERSIZE));
			}

			~Data() { this->!Data(); }
			!Data()
			{
				GC::SuppressFinalize(this);
				this->handle.Free();
				this->stream = nullptr;
				this->buffer = nullptr;
			}
		};

		int ReadFunc(void* opaque, unsigned char* buf, int buf_size);
		int WriteFunc(void* opaque, unsigned char* buf, int buf_size);
		long long SeekFunc(void* opaque, long long offset, int whence);
	}
}