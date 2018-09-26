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
				avio_closep(&pContext->pb);
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

		if (this->format_ != nullptr)
			Marshal::FreeHGlobal(static_cast<IntPtr>(const_cast<char*>(this->format_)));

	}

	void FormatContextWrapper::openRead()
	{
		if (this->opened_) throw gcnew InvalidOperationException("This FormatContextWrapper had already been opened.");
		this->opened_ = true;
		this->input_ = true;

		AVFormatContext* pFormatContext = this->formatContext = avformat_alloc_context();
		if (this->formatContext == nullptr) throw gcnew OutOfMemoryException();

		if (this->ioContextWrapper_ != nullptr)
		{
			this->ioContextWrapper_->openRead(); // Throws on failure
			formatContext->pb = this->ioContextWrapper_->ioContext;
		}

		HRESULT hr = avformat_open_input(&pFormatContext, this->file_, nullptr, nullptr);
		if (FAILED(hr)) throw gcnew AVException(hr);
	}

	void FormatContextWrapper::openWrite()
	{
		if (this->opened_) throw gcnew InvalidOperationException("This FormatContextWrapper had already been opened.");
		this->opened_ = true;

		if (this->format_ == nullptr && this->file_ == nullptr)
			throw gcnew InvalidOperationException("If wrapping an output stream, explicitly setting an output format is required.");

		AVOutputFormat* pOutputFormat = av_guess_format(this->format_, this->file_, nullptr);

		if (pOutputFormat == nullptr) return throw gcnew Exception("Could not find a suitable output format from the file name or set output format.");

		HRESULT hr = S_OK;
	
		AVFormatContext* pFormatContext = nullptr;
		if (FAILED(hr = avformat_alloc_output_context2(&pFormatContext, pOutputFormat, nullptr, this->file_)))
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

				// Wrap in another exception to keep the stacktrace.
				throw gcnew Exception("Opening the output IOContextWrapper failed.", e);
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
