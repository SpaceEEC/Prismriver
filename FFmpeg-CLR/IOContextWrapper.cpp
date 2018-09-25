#include "IOContextWrapper.h"

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

	IOContextWrapper::!IOContextWrapper()
	{
		GC::SuppressFinalize(this);

		this->handle_.Free();
		av_freep(&this->ioContext->buffer);
		pin_ptr<AVIOContext> context = this->ioContext;
		AVIOContext* pContext = context;
		av_freep(&pContext);
	}
	
	HRESULT IOContextWrapper::open(bool write)
	{
		if (this->opened_) return E_NOT_VALID_STATE;
		this->opened_ = true;
		this->write_ = write;

		// Freed in finalizer by av_freep(&this->ioContext->buffer) as libav relocates it at will
		unsigned char* buffer = static_cast<unsigned char*>(av_malloc(BUFFERSIZE));
		if (buffer == nullptr) return E_OUTOFMEMORY;

		this->ioContext = avio_alloc_context(
			buffer,
			BUFFERSIZE,
			write ? 1 : 0,
			GCHandle::ToIntPtr(this->handle_).ToPointer(),
			write ? NULL : Buffer::ReadFunc,
			write ? Buffer::WriteFunc : NULL,
			write ? NULL: Buffer::SeekFunc
		);

		if (this->ioContext == nullptr)
		{
			av_freep(&buffer);

			return E_FAIL;
		}

		return S_OK;
	}
}
