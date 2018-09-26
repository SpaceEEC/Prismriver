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
	
	void IOContextWrapper::open(bool read)
	{
		if (this->opened_) throw gcnew InvalidOperationException("This IOContextWrapper had already been opened");
		this->opened_ = true;

		// Freed in finalizer by av_freep(&this->ioContext->buffer) as libav relocates it at will
		unsigned char* buffer = static_cast<unsigned char*>(av_malloc(BUFFERSIZE));
		if (buffer == nullptr) throw gcnew OutOfMemoryException();

		Stream^ stream = static_cast<Stream^>(this->handle_.Target);

		if (read && !stream->CanRead) throw gcnew InvalidOperationException("The supplied stream is not readable.");
		if (!read && !stream->CanWrite) throw gcnew InvalidOperationException("The supplied stream is not writable.");

		this->ioContext = avio_alloc_context(
			buffer,
			BUFFERSIZE,
			stream->CanWrite ? 1 : 0,
			GCHandle::ToIntPtr(this->handle_).ToPointer(),
			stream->CanRead ? IO::ReadFunc : nullptr,
			stream->CanWrite ? IO::WriteFunc : nullptr,
			stream->CanSeek ? IO::SeekFunc : nullptr
		);

		if (this->ioContext == nullptr)
		{
			av_freep(&buffer);

			throw gcnew OutOfMemoryException();
		}
	}
}
