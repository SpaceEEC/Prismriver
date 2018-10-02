extern "C"
{
#include <libavformat/avformat.h>
}

using namespace System;
using namespace System::IO;
using namespace System::Runtime::InteropServices;

namespace Prismriver
{
	namespace IO
	{
		int ReadFunc(void* opaque, unsigned char* buf, int bufSize)
		{
			Stream^ ms = static_cast<Stream^>(static_cast<GCHandle>(static_cast<IntPtr>(opaque)).Target);
			if (!ms->CanRead) return -1;

			int chunkSize = FFMIN(bufSize, 81920);
			array<unsigned char>^ hBuffer = gcnew array<unsigned char>(chunkSize);
			int readTotal = 0;
			int read;
			try
			{
				while ((read = ms->Read(hBuffer, 0, FFMIN(chunkSize, bufSize))) != 0)
				{
					pin_ptr<unsigned char> pBuffer = &hBuffer[0];
					memcpy_s(
						buf,
						read,
						pBuffer,
						read
					);

					readTotal += read;
					buf += read;
					bufSize -= read;
				}
			}
			catch (Exception^)
			{
				return -1;
			}
			finally
			{
				delete hBuffer;
			}

			return readTotal == 0 ? AVERROR_EOF : readTotal;
		}

		int WriteFunc(void* opaque, unsigned char* buf, int bufSize)
		{
			Stream^ ms = static_cast<Stream^>(static_cast<GCHandle>(static_cast<IntPtr>(opaque)).Target);
			if (!ms->CanWrite) return -1;

			int chunkSize = FFMIN(bufSize, 81920);
			array<unsigned char>^ hBuffer = gcnew array<unsigned char>(chunkSize);
			do
			{
				int read = FFMIN(chunkSize, bufSize);

				pin_ptr<unsigned char> pBuffer = &hBuffer[0];
				memcpy_s(
					pBuffer,
					read,
					buf,
					read
				);

				try
				{
					ms->Write(hBuffer, 0, read);
				}
				catch (Exception^)
				{
					delete hBuffer;
					return -1;
				}

				buf += read;
				bufSize -= read;
			} while (bufSize != 0);

			delete hBuffer;

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