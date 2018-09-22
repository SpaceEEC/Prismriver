#include "FFmpeg-CLR.h"

#include <winerror.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavfilter/buffersrc.h>
}

#define logging(fmt, ...) av_log(NULL, AV_LOG_INFO, fmt, __VA_ARGS__)

namespace FFmpeg
{
	FFmpeg::FFmpeg(System::String^ fileIn, System::String^ fileOut)
	{
		this->storage = new Storage();
		this->fileIn = static_cast<const char*>(System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(fileIn).ToPointer());
		this->fileOut = static_cast<const char*>(System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(fileOut).ToPointer());
	}
	FFmpeg::~FFmpeg() { this->!FFmpeg(); }
	FFmpeg::!FFmpeg()
	{
		delete this->storage;
		System::Runtime::InteropServices::Marshal::FreeHGlobal(static_cast<System::IntPtr>(const_cast<char*>(this->fileIn)));
		System::Runtime::InteropServices::Marshal::FreeHGlobal(static_cast<System::IntPtr>(const_cast<char*>(this->fileOut)));
	}

	void FFmpeg::DoStuff()
	{
		HRESULT hr = S_OK;
		
		if (SUCCEEDED(hr)) hr = this->InitInput_();
		if (SUCCEEDED(hr)) hr = this->InitOutput_();
		if (SUCCEEDED(hr)) hr = this->InitFilter_();
		if (SUCCEEDED(hr)) hr = this->DoStuff_();

		if (FAILED(hr))
		{
			char errbuf[AV_ERROR_MAX_STRING_SIZE] = { 0 };
			av_strerror(hr, errbuf, AV_ERROR_MAX_STRING_SIZE);
			logging("Failure: %s", errbuf);
		}
	}

	inline HRESULT FFmpeg::InitInput_()
	{
		AVFormatContext* pFormatContext = this->storage->inputFormatContext = avformat_alloc_context();

		if (pFormatContext == nullptr) return E_OUTOFMEMORY;

		HRESULT hr = S_OK;
		if (FAILED(hr = avformat_open_input(&pFormatContext, this->fileIn, NULL, NULL))) return hr;
		logging("Found format in input: %s", (pFormatContext)->iformat->long_name);

		if (FAILED(hr = avformat_find_stream_info(pFormatContext, NULL))) return hr;

		av_dump_format(pFormatContext, 0, this->fileIn, 0);

		this->streamIndex = av_find_best_stream(pFormatContext, AVMEDIA_TYPE_AUDIO, -1, -1, &this->storage->decoder, 0);
		AVCodec* pDecoder = this->storage->decoder;

		if (this->streamIndex == AVERROR_STREAM_NOT_FOUND || pDecoder == nullptr) return E_FAIL;

		AVCodecContext* pCodecContext = this->storage->decoderContext = avcodec_alloc_context3(pDecoder);
		if (pCodecContext == nullptr)
			return E_OUTOFMEMORY;

		if (FAILED(hr = avcodec_parameters_to_context(pCodecContext, pFormatContext->streams[this->streamIndex]->codecpar)))
			return hr;

		if (FAILED(hr = avcodec_open2(pCodecContext, pDecoder, NULL)))
			return hr;

		logging("Codec name: %s", pCodecContext->codec_descriptor->long_name);

		return hr;
	}

	inline HRESULT FFmpeg::InitOutput_()
	{
		HRESULT hr = S_OK;
		if (FAILED(hr = avformat_alloc_output_context2(&this->storage->outputFormatContext, NULL, NULL, this->fileOut)))
			return hr;

		AVFormatContext* pOutputFormatContext = this->storage->outputFormatContext;

		AVStream* pOutStream = avformat_new_stream(pOutputFormatContext, NULL);

		AVCodec* pEncoder = this->storage->encoder = avcodec_find_encoder(AV_CODEC_ID_MP3);
		if (pEncoder == nullptr) return AVERROR_ENCODER_NOT_FOUND;

		AVCodecContext* pEncoderContext = this->storage->encoderContext = avcodec_alloc_context3(pEncoder);
		if (pEncoderContext == nullptr) return E_OUTOFMEMORY;

		AVCodecContext* pDecoderContext = this->storage->decoderContext;

		pEncoderContext->sample_rate = pDecoderContext->sample_rate;
		pEncoderContext->channel_layout = pDecoderContext->channel_layout;
		pEncoderContext->channels = av_get_channel_layout_nb_channels(pEncoderContext->channel_layout);

		pEncoderContext->sample_fmt = pEncoder->sample_fmts[0];
		pEncoderContext->time_base.den = 1;
		pEncoderContext->time_base.num = pEncoderContext->sample_rate;

		if (FAILED(hr = avcodec_open2(pEncoderContext, pEncoder, NULL)))
			return hr;

		if (FAILED(hr = avcodec_parameters_from_context(pOutStream->codecpar, pEncoderContext)))
			return hr;

		if (pOutputFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
			pEncoderContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

		pOutStream->time_base = pEncoderContext->time_base;

		av_dump_format(pOutputFormatContext, 0, this->fileOut, 1);

		if (!(pOutputFormatContext->oformat->flags & AVFMT_NOFILE))
			if (FAILED(hr = avio_open(&pOutputFormatContext->pb, this->fileOut, AVIO_FLAG_WRITE)))
				return hr;

		if (FAILED(hr = avformat_write_header(pOutputFormatContext, NULL)))
			return hr;

		return S_OK;
	}

	inline HRESULT FFmpeg::InitFilter_()
	{
		const AVFilter* pBufferSource = NULL;
		const AVFilter* pBufferSink = NULL;

		AVFilterInOut* pOutputs = avfilter_inout_alloc();
		AVFilterInOut* pInputs = avfilter_inout_alloc();
		try
		{
			AVFilterGraph* pFilterGraph = this->storage->filterGraph = avfilter_graph_alloc();

			if (pOutputs == nullptr || pInputs == nullptr || pFilterGraph == nullptr)
				return E_OUTOFMEMORY;

			pBufferSource = avfilter_get_by_name("abuffer");
			pBufferSink = avfilter_get_by_name("abuffersink");

			// Should never happen
			if (pBufferSource == nullptr || pBufferSink == nullptr)
				return AVERROR_UNKNOWN;

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
			if (FAILED(hr = avfilter_graph_create_filter(&this->storage->bufferSourceContext, pBufferSource, "in", args, NULL, pFilterGraph)))
				return hr;
			AVFilterContext* pBufferSourceContext = this->storage->bufferSourceContext;


			if (FAILED(hr = avfilter_graph_create_filter(&this->storage->bufferSinkContext, pBufferSink, "out", NULL, NULL, pFilterGraph)))
				return hr;
			AVFilterContext* pBufferSinkContext = this->storage->bufferSinkContext;

			AVCodecContext* pEncoderContext = this->storage->encoderContext;

			hr = av_opt_set_bin(
				pBufferSinkContext,
				"sample_fmts",
				reinterpret_cast<uint8_t*>(&pEncoderContext->sample_fmt),
				sizeof(pEncoderContext->sample_fmt),
				AV_OPT_SEARCH_CHILDREN
			);
			if (FAILED(hr)) return hr;

			hr = av_opt_set_bin(
				pBufferSinkContext,
				"channel_layouts",
				reinterpret_cast<uint8_t*>(&pEncoderContext->channel_layout),
				sizeof(pEncoderContext->channel_layout),
				AV_OPT_SEARCH_CHILDREN
			);
			if (FAILED(hr)) return hr;

			hr = av_opt_set_bin(
				pBufferSinkContext,
				"sample_rates",
				reinterpret_cast<uint8_t*>(&pEncoderContext->sample_rate),
				sizeof(pEncoderContext->sample_rate),
				AV_OPT_SEARCH_CHILDREN
			);
			if (FAILED(hr)) return hr;

			pOutputs->name = av_strdup("in");
			pOutputs->filter_ctx = pBufferSourceContext;
			pOutputs->pad_idx = 0;
			pOutputs->next = NULL;

			pInputs->name = av_strdup("out");
			pInputs->filter_ctx = pBufferSinkContext;
			pInputs->pad_idx = 0;
			pInputs->next = NULL;

			if (pOutputs->name == nullptr || pInputs->name == nullptr)
				return E_OUTOFMEMORY;

			if (FAILED(hr = avfilter_graph_parse_ptr(pFilterGraph, "anull", &pInputs, &pOutputs, NULL)))
				return hr;

			if (FAILED(hr = avfilter_graph_config(pFilterGraph, NULL)))
				return hr;

			av_buffersink_set_frame_size(pBufferSinkContext, pEncoderContext->frame_size);
		}
		finally
		{
			avfilter_inout_free(&pInputs);
			avfilter_inout_free(&pOutputs);
		}

		return S_OK;
	}
	
	inline HRESULT FFmpeg::DoStuff_()
	{
		AVFrame* pFrame = av_frame_alloc();
		if (pFrame == nullptr) return E_OUTOFMEMORY;

		AVFrame* pFilterFrame = av_frame_alloc();
		if (pFilterFrame == nullptr) return E_OUTOFMEMORY;

		AVPacket* pPacket = av_packet_alloc();
		if (pPacket == nullptr)
		{
			av_frame_free(&pFrame);
			av_frame_free(&pFilterFrame);

			return E_OUTOFMEMORY;
		}

		HRESULT hr = S_OK;

		while (SUCCEEDED(av_read_frame(this->storage->inputFormatContext, pPacket)))
		{
			if (pPacket->stream_index == this->streamIndex) 
				this->DecodePacket_(pPacket, pFrame, pFilterFrame);
			av_packet_unref(pPacket);
		}

		this->FilterFrame_(NULL, pFilterFrame);

		if (this->storage->encoderContext->codec->capabilities & AV_CODEC_CAP_DELAY)
			this->EncodeWriteFrame_(NULL);

		av_packet_free(&pPacket);
		av_frame_free(&pFilterFrame);
		av_frame_free(&pFrame);

		return hr;
	}

	inline void FFmpeg::DecodePacket_(AVPacket* pPacket, AVFrame* pFrame, AVFrame* pFilterFrame)
	{
		AVCodecContext* pDecoderContext = this->storage->decoderContext;

		av_packet_rescale_ts(
			pPacket,
			this->storage->inputFormatContext->streams[this->streamIndex]->time_base,
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

	inline void FFmpeg::FilterFrame_(AVFrame* pFrame, AVFrame* pFilterFrame)
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

	inline void FFmpeg::EncodeWriteFrame_(AVFrame* pFilterFrame)
	{
		AVPacket* encodedPacket = av_packet_alloc();

		avcodec_send_frame(this->storage->encoderContext, pFilterFrame);

		do
		{
			if (FAILED(avcodec_receive_packet(this->storage->encoderContext, encodedPacket)))
				break;

			av_packet_rescale_ts(encodedPacket, this->storage->encoderContext->time_base, this->storage->decoderContext->time_base);

			if (FAILED(av_interleaved_write_frame(this->storage->outputFormatContext, encodedPacket)))
				break;

			av_packet_unref(encodedPacket);
		} while (true);

		av_packet_unref(encodedPacket);
		av_free_packet(encodedPacket);
	}
}