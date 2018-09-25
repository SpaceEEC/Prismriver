#include "FormatContextWrapper.h"

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

	HRESULT FormatContextWrapper::openRead()
	{
		if (this->opened_) return E_NOT_VALID_STATE;
		this->input_ = true;

		HRESULT hr = S_OK;
		if (this->ioContextWrapper_ != nullptr && FAILED(hr = this->ioContextWrapper_->openRead()))
			return hr;

		AVFormatContext* pFormatContext = this->formatContext = avformat_alloc_context();
		if (this->formatContext == nullptr) return E_OUTOFMEMORY;

		if (this->ioContextWrapper_ != nullptr) formatContext->pb = this->ioContextWrapper_->ioContext;

		return avformat_open_input(&pFormatContext, this->file_, NULL, NULL);
	}

	HRESULT FormatContextWrapper::openWrite()
	{
		if (this->opened_) return E_NOT_VALID_STATE;

		AVOutputFormat* pOutputFormat = av_guess_format("mp3", NULL, NULL);
		if (pOutputFormat == nullptr) return E_UNEXPECTED;

		HRESULT hr = S_OK;
		if (this->ioContextWrapper_ != nullptr && FAILED(hr = this->ioContextWrapper_->openWrite()))
			return hr;
			
		AVFormatContext* pFormatContext = nullptr;
		if (FAILED(hr = avformat_alloc_output_context2(&pFormatContext, pOutputFormat, NULL, this->file_)))
			return hr;
		this->formatContext = pFormatContext;

		if (this->ioContextWrapper_ != nullptr) this->formatContext->pb = this->ioContextWrapper_->ioContext;
		
		return hr;
	}
}
