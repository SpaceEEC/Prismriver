#include "FormatContextWrapper.h"
#include "AVException.h"

namespace FFmpeg
{
	FormatContextWrapper::!FormatContextWrapper()
	{
		GC::SuppressFinalize(this);

		if (this->formatContext != nullptr)
		{
			pin_ptr<AVFormatContext> context = this->formatContext;
			AVFormatContext* pContext = context;
			if (this->input_)
			{
				avformat_close_input(&pContext);
			}
			else
			{
				if (this->file_ != nullptr) avio_closep(&pContext->pb);

				avformat_free_context(pContext);
				this->formatContext = nullptr;
			}
		}

		if (this->file_ != nullptr)
		{
			Marshal::FreeHGlobal(static_cast<IntPtr>(const_cast<char*>(this->file_)));
			this->file_ = nullptr;
		}

		if (this->ioContextWrapper_ != nullptr)
		{
			delete this->ioContextWrapper_;
			this->ioContextWrapper_ = nullptr;
		}
	}

	void FormatContextWrapper::openRead()
	{
		if (this->opened_) throw gcnew InvalidOperationException("This FormatContextWrapper had already been opened.");
		this->input_ = true;

		if (this->ioContextWrapper_ != nullptr) this->ioContextWrapper_->openRead(); // Throws on failure

		AVFormatContext* pFormatContext = this->formatContext = avformat_alloc_context();
		if (this->formatContext == nullptr) throw gcnew OutOfMemoryException();

		if (this->ioContextWrapper_ != nullptr) formatContext->pb = this->ioContextWrapper_->ioContext;

		HRESULT hr = avformat_open_input(&pFormatContext, this->file_, NULL, NULL);
		if (FAILED(hr))
			throw gcnew AVException(hr);
	}

	void FormatContextWrapper::openWrite()
	{
		if (this->opened_) throw gcnew InvalidOperationException("This FormatContextWrapper had already been opened.");

		AVOutputFormat* pOutputFormat = av_guess_format(this->file_ == nullptr ? "flac" : NULL, this->file_, NULL);
		if (pOutputFormat == nullptr) return throw gcnew Exception("Could not find a suitable output format");

		HRESULT hr = S_OK;
	
		AVFormatContext* pFormatContext = nullptr;
		if (FAILED(hr = avformat_alloc_output_context2(&pFormatContext, pOutputFormat, NULL, this->file_)))
			throw gcnew AVException(hr);

		if (this->ioContextWrapper_ != nullptr)
		{
			try
			{
				this->ioContextWrapper_->openWrite();
			}
			catch (Exception^ e)
			{
				avformat_free_context(pFormatContext);

				throw e;
			}

			pFormatContext->pb = this->ioContextWrapper_->ioContext;
		}
		else if ((pOutputFormat->flags & AVFMT_NOFILE) == 0)
		{
			if (FAILED(hr = avio_open(&pFormatContext->pb, this->file_, AVIO_FLAG_WRITE)))
			{
				avformat_free_context(pFormatContext);

				throw gcnew AVException(hr);
			}
		}

		this->formatContext = pFormatContext;
	}
}
