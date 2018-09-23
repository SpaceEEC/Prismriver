#include "BufferData.h"

#include <stdint.h>

#include <algorithm>
#include <iterator>

#include <libavformat/avformat.h>

namespace FFmpeg
{
	namespace Buffer
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

			hBuffer = nullptr;

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
				hBuffer = nullptr;
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
