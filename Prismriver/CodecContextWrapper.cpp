#include "CodecContextWrapper.h"

namespace Prismriver
{
	CodecContextWrapper::~CodecContextWrapper()
	{
		FormatContextWrapper::~FormatContextWrapper();

		this->streamIndex = AVERROR_STREAM_NOT_FOUND;
		this->codec = nullptr;
		avcodec_free_context(&this->codecContext);
	}

	AVStream* CodecContextWrapper::GetStream()
	{
		if (this->streamIndex == AVERROR_STREAM_NOT_FOUND) return nullptr;
		if (this->formatContext == nullptr) return nullptr;
		return this->formatContext->streams[this->streamIndex];
	}

	void CodecContextWrapper::OpenRead()
	{
		FormatContextWrapper::OpenRead();

		HRESULT hr = avformat_find_stream_info(this->formatContext, nullptr);
		if (FAILED(hr)) throw gcnew AVException(hr);

		av_dump_format(this->formatContext, 0, this->file_, 0);

		this->streamIndex = av_find_best_stream(this->formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, &this->codec, 0);
		if (this->streamIndex == AVERROR_STREAM_NOT_FOUND || this->codec == nullptr)
			throw gcnew AVException(AVERROR_STREAM_NOT_FOUND);

		this->codecContext = avcodec_alloc_context3(this->codec);
		if (this->codecContext == nullptr) throw gcnew OutOfMemoryException();

		hr = avcodec_parameters_to_context(this->codecContext, this->formatContext->streams[this->streamIndex]->codecpar);
		if (FAILED(hr)) throw gcnew AVException(hr);

		hr = avcodec_open2(this->codecContext, this->codec, nullptr);
		if (FAILED(hr)) throw gcnew AVException(hr);
	}

	void CodecContextWrapper::OpenWrite(CodecContextWrapper* dataIn)
	{
		FormatContextWrapper::OpenWrite();

		this->codec = avcodec_find_encoder(this->formatContext->oformat->audio_codec);
		if (this->codec == nullptr) throw gcnew AVException(AVERROR_ENCODER_NOT_FOUND);

		AVStream* pStream = avformat_new_stream(this->formatContext, this->codec);
		if (pStream == nullptr) throw gcnew OutOfMemoryException();

		AVCodecContext* cOut = this->codecContext = avcodec_alloc_context3(this->codec);
		if (this->codecContext == nullptr) throw gcnew OutOfMemoryException();

		AVCodecContext* cIn = dataIn->codecContext;

		cOut->bit_rate = dataIn->formatContext->bit_rate;
		cOut->time_base.den = cOut->sample_rate = cIn->sample_rate;
		cOut->channel_layout = cIn->channel_layout;
		cOut->channels = av_get_channel_layout_nb_channels(cOut->channel_layout);
		cOut->sample_fmt = this->codec->sample_fmts[0];
		cOut->time_base.num = 1;

		pStream->time_base = cOut->time_base;

		HRESULT hr = avcodec_open2(this->codecContext, this->codec, nullptr);
		if (FAILED(hr)) throw gcnew AVException(hr);

		hr = avcodec_parameters_from_context(pStream->codecpar, this->codecContext);
		if (FAILED(hr)) throw gcnew AVException(hr);

		if ((this->formatContext->oformat->flags & AVFMT_GLOBALHEADER) != 0)
			this->codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

		av_dump_format(this->formatContext, 0, this->file_, 1);
	}
}
