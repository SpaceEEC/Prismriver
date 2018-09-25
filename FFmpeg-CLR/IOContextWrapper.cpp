#include "IOContextWrapper.h"

#include "IO.h"

namespace FFmpeg
{
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
			write ? NULL : IO::ReadFunc,
			write ? IO::WriteFunc : NULL,
			write ? NULL: IO::SeekFunc
		);

		if (this->ioContext == nullptr)
		{
			av_freep(&buffer);

			return E_FAIL;
		}

		return S_OK;
	}
}
