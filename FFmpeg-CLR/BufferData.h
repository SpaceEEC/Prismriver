#pragma once

using namespace System;
using namespace System::IO;
using namespace System::Runtime::InteropServices;

namespace FFmpeg
{
	namespace Buffer
	{
		private ref struct Data
		{
		private:
			GCHandle handle;

		public:
			void* ptr;

			Data(Stream^ stream) : handle(GCHandle::Alloc(stream))
			{
				this->ptr = GCHandle::ToIntPtr(this->handle).ToPointer();
			}

			~Data()
			{
				this->handle.Free();
			}
		};

		int ReadFunc(void* opaque, unsigned char* buf, int buf_size);
		int WriteFunc(void* opaque, unsigned char* buf, int buf_size);
		long long SeekFunc(void* opaque, long long offset, int whence);
	}
}