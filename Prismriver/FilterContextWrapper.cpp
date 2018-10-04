#include "FilterContextWrapper.h"
#include "Handle.h"

extern "C"
{
#include "libavutil/opt.h"
}

namespace Prismriver
{
	FilterContextWrapper::~FilterContextWrapper()
	{
		avfilter_free(this->bufferSourceContext);
		avfilter_free(this->bufferSinkContext);

		avfilter_graph_free(&this->filterGraph);
	}
	void FilterContextWrapper::InitFilters(CodecContextWrapper* dataIn)
	{
		const AVFilter* pBufferSource = nullptr;
		const AVFilter* pBufferSink = nullptr;

		InOutHandle outputs(avfilter_inout_alloc(), avfilter_inout_free);
		InOutHandle inputs(avfilter_inout_alloc(), avfilter_inout_free);
		AVFilterGraph* pFilterGraph = this->filterGraph = avfilter_graph_alloc();

		if (!outputs.IsValid() || !inputs.IsValid() || pFilterGraph == nullptr)
			throw gcnew OutOfMemoryException();

		pBufferSource = avfilter_get_by_name("abuffer");
		pBufferSink = avfilter_get_by_name("abuffersink");

		if (pBufferSource == nullptr || pBufferSink == nullptr)
			throw gcnew AVException("Could not find \"abuffer\" and / or \"abuffersink\" filters.\n This should never happen.");

		AVCodecContext* pDecoderContext = dataIn->codecContext;

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

		HRESULT hr = avfilter_graph_create_filter(&this->bufferSourceContext, pBufferSource, "in", args, nullptr, pFilterGraph);
		if (FAILED(hr)) throw gcnew AVException(hr);

		hr = avfilter_graph_create_filter(&this->bufferSinkContext, pBufferSink, "out", nullptr, nullptr, pFilterGraph);
		if (FAILED(hr)) throw gcnew AVException(hr);

		AVFilterContext* pBufferSinkContext = this->bufferSinkContext;
		AVCodecContext* pEncCtx = this->codecContext;

		hr = av_opt_set_bin(
			pBufferSinkContext,
			"sample_fmts",
			reinterpret_cast<unsigned char*>(&pEncCtx->sample_fmt),
			sizeof(pEncCtx->sample_fmt),
			AV_OPT_SEARCH_CHILDREN
		);
		if (FAILED(hr)) throw gcnew AVException(hr);

		hr = av_opt_set_bin(
			pBufferSinkContext,
			"channel_layouts",
			reinterpret_cast<unsigned char*>(&pEncCtx->channel_layout),
			sizeof(pEncCtx->channel_layout),
			AV_OPT_SEARCH_CHILDREN
		);
		if (FAILED(hr)) throw gcnew AVException(hr);

		hr = av_opt_set_bin(
			pBufferSinkContext,
			"sample_rates",
			reinterpret_cast<unsigned char*>(&pEncCtx->sample_rate),
			sizeof(pEncCtx->sample_rate),
			AV_OPT_SEARCH_CHILDREN
		);
		if (FAILED(hr)) throw gcnew AVException(hr);

		outputs->name = av_strdup("in");
		outputs->filter_ctx = this->bufferSourceContext;
		outputs->pad_idx = 0;
		outputs->next = nullptr;

		inputs->name = av_strdup("out");
		inputs->filter_ctx = pBufferSinkContext;
		inputs->pad_idx = 0;
		inputs->next = nullptr;

		if (outputs->name == nullptr || inputs->name == nullptr)
			throw gcnew OutOfMemoryException();

		hr = avfilter_graph_parse_ptr(pFilterGraph, "anull", &inputs, &outputs, nullptr);
		if (FAILED(hr)) throw gcnew AVException(hr);

		hr = avfilter_graph_config(pFilterGraph, nullptr);
		if (FAILED(hr)) throw gcnew AVException(hr);

		av_buffersink_set_frame_size(pBufferSinkContext, pEncCtx->frame_size);
	}
}