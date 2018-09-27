#include "Transcoder.h"
#include "FormatContextWrapper.h"
#include "AVException.h"

#include <winerror.h>

extern "C"
{
#include <libavutil/opt.h>
#include <libavfilter/buffersrc.h>
}

#define logging(fmt, ...) av_log(nullptr, AV_LOG_INFO, fmt, __VA_ARGS__)

namespace FFmpeg
{
	Transcoder::Transcoder() : storage(new Storage()) {}
	Transcoder::~Transcoder() { this->!Transcoder(); }
	Transcoder::!Transcoder()
	{
		GC::SuppressFinalize(this);
		delete this->format_;
		delete this->storage;
		delete this->dataIn;
		delete this->dataOut;
	}

	Transcoder^ Transcoder::SetIn(Stream^ stream)
	{
		if (this->dataIn != nullptr) delete this->dataIn;

		this->dataIn = new FormatContextWrapper(stream);

		return this;
	}
	Transcoder^ Transcoder::SetIn(String^ string)
	{
		if (this->dataIn != nullptr) delete this->dataOut;

		this->dataIn = new FormatContextWrapper(string);
		return this;
	}

	Transcoder^ Transcoder::SetOut(Stream^ stream)
	{
		if (this->dataOut != nullptr) delete this->dataOut;

		this->dataOut = new FormatContextWrapper(stream);

		if (this->format_) this->dataOut->setOutFormat(this->format_);

		return this;
	}
	Transcoder^ Transcoder::SetOut(String^ string)
	{
		if (this->dataOut != nullptr) delete this->dataOut;

		this->dataOut = new FormatContextWrapper(string);

		if (this->format_) this->dataOut->setOutFormat(this->format_);

		return this;
	}

	Transcoder^ Transcoder::SetOutFormat(String^ format)
	{
		this->format_ = format;

		if (this->dataOut) this->dataOut->setOutFormat(this->format_);

		return this;
	}

	void Transcoder::DoStuff()
	{
		this->InitInput_();
		this->InitOutput_();
		this->InitFilter_();
		this->DoStuff_();
	}

	inline void Transcoder::InitInput_()
	{
		this->dataIn->openRead();

		AVFormatContext* pFormatContext = this->dataIn->formatContext;

		logging("Found format in input: %s", pFormatContext->iformat->long_name);

		HRESULT hr = S_OK;
		if (FAILED(hr = avformat_find_stream_info(pFormatContext, nullptr))) throw gcnew AVException(hr);

		av_dump_format(pFormatContext, 0, nullptr, 0);

		this->streamIndex = av_find_best_stream(pFormatContext, AVMEDIA_TYPE_AUDIO, -1, -1, &this->storage->decoder, 0);
		AVCodec* pDecoder = this->storage->decoder;

		if (this->streamIndex == AVERROR_STREAM_NOT_FOUND || pDecoder == nullptr) throw gcnew AVException(AVERROR_STREAM_NOT_FOUND);

		AVCodecContext* pCodecContext = this->storage->decoderContext = avcodec_alloc_context3(pDecoder);
		if (pCodecContext == nullptr)
			throw gcnew OutOfMemoryException();

		if (FAILED(hr = avcodec_parameters_to_context(pCodecContext, pFormatContext->streams[this->streamIndex]->codecpar)))
			throw gcnew AVException(hr);

		if (FAILED(hr = avcodec_open2(pCodecContext, pDecoder, nullptr)))
			throw gcnew AVException(hr);

		logging("Codec name: %s", pCodecContext->codec_descriptor->long_name);
	}

	inline void Transcoder::InitOutput_()
	{
		this->dataOut->openWrite();

		AVFormatContext* pOutputFormatContext = this->dataOut->formatContext;

		AVStream* pOutStream = avformat_new_stream(pOutputFormatContext, nullptr);

		AVCodec* pEncoder = this->storage->encoder = avcodec_find_encoder(pOutputFormatContext->oformat->audio_codec);
		if (pEncoder == nullptr) throw gcnew AVException(AVERROR_ENCODER_NOT_FOUND);

		AVCodecContext* pEncoderContext = this->storage->encoderContext = avcodec_alloc_context3(pEncoder);
		if (pEncoderContext == nullptr) throw gcnew OutOfMemoryException();

		AVCodecContext* pDecoderContext = this->storage->decoderContext;

		pEncoderContext->bit_rate = pDecoderContext->bit_rate;
		pEncoderContext->sample_rate = pDecoderContext->sample_rate;
		pEncoderContext->channel_layout = pDecoderContext->channel_layout;
		pEncoderContext->channels = av_get_channel_layout_nb_channels(pEncoderContext->channel_layout);

		pEncoderContext->sample_fmt = pEncoder->sample_fmts[0];
		pEncoderContext->time_base.den = 1;
		pEncoderContext->time_base.num = pEncoderContext->sample_rate;

		HRESULT hr = S_OK;
		if (FAILED(hr = avcodec_open2(pEncoderContext, pEncoder, nullptr)))
			throw gcnew AVException(hr);

		if (FAILED(hr = avcodec_parameters_from_context(pOutStream->codecpar, pEncoderContext)))
			throw gcnew AVException(hr);

		if (pOutputFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
			pEncoderContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

		pOutStream->time_base = pEncoderContext->time_base;

		av_dump_format(pOutputFormatContext, 0, nullptr, 1);

		if (FAILED(hr = avformat_write_header(pOutputFormatContext, nullptr)))
			throw gcnew AVException(hr);
	}

	inline void Transcoder::InitFilter_()
	{
		const AVFilter* pBufferSource = nullptr;
		const AVFilter* pBufferSink = nullptr;

		AVFilterInOut* pOutputs = avfilter_inout_alloc();
		AVFilterInOut* pInputs = avfilter_inout_alloc();
		try
		{
			AVFilterGraph* pFilterGraph = this->storage->filterGraph = avfilter_graph_alloc();

			if (pOutputs == nullptr || pInputs == nullptr || pFilterGraph == nullptr)
				throw gcnew OutOfMemoryException();

			pBufferSource = avfilter_get_by_name("abuffer");
			pBufferSink = avfilter_get_by_name("abuffersink");

			if (pBufferSource == nullptr || pBufferSink == nullptr)
				throw gcnew AVException("Could not find \"abuffer\" and / or \"abuffersink\" filters.\n This should never happen.");

			AVCodecContext* pDecoderContext = this->storage->decoderContext;

			if (!pDecoderContext->channel_layout)
				pDecoderContext->channel_layout = av_get_default_channel_layout(pDecoderContext->channels);

			char args[512];

			snprintf(
				args,
				sizeof(args),
				"time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%llx",
				pDecoderContext->time_base.num,
				pDecoderContext->time_base.den,
				pDecoderContext->sample_rate,
				av_get_sample_fmt_name(pDecoderContext->sample_fmt),
				pDecoderContext->channel_layout
			);
			

			HRESULT hr = S_OK;
			if (FAILED(hr = avfilter_graph_create_filter(&this->storage->bufferSourceContext, pBufferSource, "in", args, nullptr, pFilterGraph)))
				throw gcnew AVException(hr);
			AVFilterContext* pBufferSourceContext = this->storage->bufferSourceContext;


			if (FAILED(hr = avfilter_graph_create_filter(&this->storage->bufferSinkContext, pBufferSink, "out", nullptr, nullptr, pFilterGraph)))
				throw gcnew AVException(hr);

			AVFilterContext* pBufferSinkContext = this->storage->bufferSinkContext;
			AVCodecContext* pEncoderContext = this->storage->encoderContext;

			hr = av_opt_set_bin(
				pBufferSinkContext,
				"sample_fmts",
				reinterpret_cast<uint8_t*>(&pEncoderContext->sample_fmt),
				sizeof(pEncoderContext->sample_fmt),
				AV_OPT_SEARCH_CHILDREN
			);
			if (FAILED(hr)) throw gcnew AVException(hr);


			hr = av_opt_set_bin(
				pBufferSinkContext,
				"channel_layouts",
				reinterpret_cast<uint8_t*>(&pEncoderContext->channel_layout),
				sizeof(pEncoderContext->channel_layout),
				AV_OPT_SEARCH_CHILDREN
			);
			if (FAILED(hr)) throw gcnew AVException(hr);

			hr = av_opt_set_bin(
				pBufferSinkContext,
				"sample_rates",
				reinterpret_cast<uint8_t*>(&pEncoderContext->sample_rate),
				sizeof(pEncoderContext->sample_rate),
				AV_OPT_SEARCH_CHILDREN
			);
			if (FAILED(hr)) throw gcnew AVException(hr);

			pOutputs->name = av_strdup("in");
			pOutputs->filter_ctx = pBufferSourceContext;
			pOutputs->pad_idx = 0;
			pOutputs->next = nullptr;

			pInputs->name = av_strdup("out");
			pInputs->filter_ctx = pBufferSinkContext;
			pInputs->pad_idx = 0;
			pInputs->next = nullptr;

			if (pOutputs->name == nullptr || pInputs->name == nullptr)
				throw gcnew OutOfMemoryException();

			if (FAILED(hr = avfilter_graph_parse_ptr(pFilterGraph, "anull", &pInputs, &pOutputs, nullptr)))
				throw gcnew AVException(hr);

			if (FAILED(hr = avfilter_graph_config(pFilterGraph, nullptr)))
				throw gcnew AVException(hr);

			av_buffersink_set_frame_size(pBufferSinkContext, pEncoderContext->frame_size);
		}
		finally
		{
			avfilter_inout_free(&pInputs);
			avfilter_inout_free(&pOutputs);
		}
	}
	
	inline void Transcoder::DoStuff_()
	{
		AVFrame* pFrame = av_frame_alloc();
		if (pFrame == nullptr) throw gcnew OutOfMemoryException();

		AVFrame* pFilterFrame = av_frame_alloc();
		if (pFilterFrame == nullptr)
		{
			av_frame_free(&pFrame);

			throw gcnew OutOfMemoryException();
		}

		AVPacket* pPacket = av_packet_alloc();
		if (pPacket == nullptr)
		{
			av_frame_free(&pFrame);
			av_frame_free(&pFilterFrame);

			throw gcnew OutOfMemoryException();
		}

		try
		{
			while (SUCCEEDED(av_read_frame(this->dataIn->formatContext, pPacket)))
			{
				if (pPacket->stream_index == this->streamIndex)
					this->DecodePacket_(pPacket, pFrame, pFilterFrame);
				av_packet_unref(pPacket);
			}

			this->FilterFrame_(nullptr, pFilterFrame);

			this->EncodeWriteFrame_(nullptr);

			HRESULT hr = S_OK;
			if (FAILED(hr = av_write_trailer(this->dataOut->formatContext)))
				throw gcnew AVException(hr);
		}
		finally
		{
			av_packet_free(&pPacket);
			av_frame_free(&pFilterFrame);
			av_frame_free(&pFrame);
		}
	}

	inline void Transcoder::DecodePacket_(AVPacket* pPacket, AVFrame* pFrame, AVFrame* pFilterFrame)
	{
		AVCodecContext* pDecoderContext = this->storage->decoderContext;

		av_packet_rescale_ts(
			pPacket,
			this->dataIn->formatContext->streams[this->streamIndex]->time_base,
			pDecoderContext->time_base
		);

		avcodec_send_packet(pDecoderContext, pPacket);

		do
		{
			if (FAILED(avcodec_receive_frame(pDecoderContext, pFrame)))
				return;

			pFrame->pts = pFrame->best_effort_timestamp;

			this->FilterFrame_(pFrame, pFilterFrame);

			av_frame_unref(pFrame);
		} while (true);
	}

	inline void Transcoder::FilterFrame_(AVFrame* pFrame, AVFrame* pFilterFrame)
	{
		av_buffersrc_add_frame_flags(this->storage->bufferSourceContext, pFrame, 0);

		do
		{
			if (FAILED(av_buffersink_get_frame(this->storage->bufferSinkContext, pFilterFrame)))
				return;

			pFilterFrame->pict_type = AV_PICTURE_TYPE_NONE;

			this->EncodeWriteFrame_(pFilterFrame);

			av_frame_unref(pFilterFrame);
		} while (true);
	}

	inline void Transcoder::EncodeWriteFrame_(AVFrame* pFilterFrame)
	{
		AVPacket* encodedPacket = av_packet_alloc();

		avcodec_send_frame(this->storage->encoderContext, pFilterFrame);

		do
		{
			if (FAILED(avcodec_receive_packet(this->storage->encoderContext, encodedPacket)))
				break;

			av_packet_rescale_ts(encodedPacket, this->storage->encoderContext->time_base, this->storage->decoderContext->time_base);

			if (FAILED(av_interleaved_write_frame(this->dataOut->formatContext, encodedPacket)))
				break;

			av_packet_unref(encodedPacket);
		} while (true);

		av_packet_unref(encodedPacket);
	}
}