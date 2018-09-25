extern "C"
{
#include <libavformat/avformat.h>
}

using namespace System;
using namespace System::IO;
using namespace System::Runtime::InteropServices;

namespace FFmpeg
{
	namespace IO
	{
		int ReadFunc(void* opaque, unsigned char* buf, int bufSize)
		{
			Stream^ ms = static_cast<Stream^>(static_cast<GCHandle>(static_cast<IntPtr>(opaque)).Target);
			if (!ms->CanRead) return -1;

			array<unsigned char>^ hBuffer = gcnew array<unsigned char>(bufSize);

			int read = ms->Read(hBuffer, 0, bufSize);

			{
				pin_ptr<unsigned char> pinned = &hBuffer[0];
				memcpy_s(
					buf,
					sizeof(unsigned char) * read,
					static_cast<void*>(pinned),
					sizeof(unsigned char) * read
				);
			}

			delete hBuffer;

			return read == 0 ? AVERROR_EOF : read;
		}

		int WriteFunc(void* opaque, unsigned char* buf, int bufSize)
		{
			Stream^ ms = static_cast<Stream^>(static_cast<GCHandle>(static_cast<IntPtr>(opaque)).Target);
			if (!ms->CanWrite) return -1;

			array<unsigned char>^ hBuffer = gcnew array<unsigned char>(bufSize);

			{
				pin_ptr<unsigned char> pinned = &hBuffer[0];
				memcpy_s(
					static_cast<void*>(pinned),
					bufSize,
					buf,
					bufSize
				);
			}

			try
			{
				ms->Write(hBuffer, 0, bufSize);
			}
			catch (Exception^)
			{
				return -1;
			}
			finally
			{
				delete hBuffer;
			}

			return 0;
		}

		long long SeekFunc(void* opaque, long long offset, int whence)
		{
			Stream^ ms = static_cast<Stream^>(static_cast<GCHandle>(static_cast<IntPtr>(opaque)).Target);

			try
			{
				if ((whence & AVSEEK_SIZE) == AVSEEK_SIZE) return ms->Length;
				if (!ms->CanSeek) return -1;

				return ms->Seek(offset, static_cast<SeekOrigin>(whence));
			}
			catch (Exception^)
			{
				return -1;
			}
		}
	}
}