#include "AVException.h"
#include "AVHandle.h"
#include "FormatContextWrapper.h"
#include "Transcoder.h"
#include "Utils.h"

#include <winerror.h>

extern "C"
{
#include <libavutil/opt.h>
#include <libavfilter/buffersrc.h>
}

#define logging(fmt, ...) av_log(nullptr, AV_LOG_INFO, fmt, __VA_ARGS__)

namespace FFmpeg
{
	Transcoder::Transcoder(Stream^ stream, array<ITrack^>^ tracks)
		: dataIn(new CodecContextWrapper(stream)),
		tracks(tracks) {}

	Transcoder::Transcoder(String^ file, array<ITrack^>^ tracks)
		: dataIn(new CodecContextWrapper(file)),
		tracks(tracks) {}

	Transcoder::~Transcoder() { this->!Transcoder(); }
	Transcoder::!Transcoder()
	{
		GC::SuppressFinalize(this);
		delete this->storage;
		delete this->dataIn;
	}

	void Transcoder::Run()
	{
		this->dataIn->openRead();
		this->storage = new Storage();

		for each(ITrack^ track in this->tracks)
		{
			// TODO: Dedup this
			Stream^ stream = dynamic_cast<Stream^>(track->Target);
			if (stream != nullptr)
			{
				CodecContextWrapper out(stream);
				// TODO: sane api for this
				out.setOutFormat("flac");
				out.openWrite(this->dataIn);

				AVDictionary* meta = out.formatContext->metadata;

				HRESULT hr = av_dict_copy(&meta, dataIn->formatContext->metadata, AV_DICT_IGNORE_SUFFIX);
				if (FAILED(hr)) throw gcnew AVException(hr);

				if (track->Album != nullptr)
					av_dict_set(
						&meta,
						"album",
						Utils::stringtoUtf8(track->Album),
						// TODO: those strings shouldn't be duplicated, but freeing those causes a heap corruption?
						0 //AV_DICT_DONT_STRDUP_VAL
					);

				if (track->Author != nullptr)
					av_dict_set(
						&meta,
						"artist",
						Utils::stringtoUtf8(track->Author),
						0 //AV_DICT_DONT_STRDUP_VAL
					);

				if (track->Title != nullptr)
					av_dict_set(
						&meta,
						"title",
						Utils::stringtoUtf8(track->Title),
						0 //AV_DICT_DONT_STRDUP_VAL
					);

				out.formatContext->metadata = meta;
				av_dict_copy(&out.formatContext->streams[0]->metadata, meta, AV_DICT_IGNORE_SUFFIX);

				hr = avformat_write_header(out.formatContext, nullptr);
				if (FAILED(hr)) throw gcnew AVException(hr);

				AVDictionaryEntry* tag = nullptr;
				while ((tag = av_dict_get(out.formatContext->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
					logging("key %s -> value %s\n", tag->key, tag->value);

				while ((tag = av_dict_get(out.formatContext->streams[0]->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
					logging("key %s -> value %s\n", tag->key, tag->value);

				this->InitFilter_(out);
				this->Run_(out, track);

				continue;
			}

			String^ file = dynamic_cast<String^>(track->Target);
			if (file != nullptr)
			{
				CodecContextWrapper out(file);
				out.openWrite(this->dataIn);

				this->InitFilter_(out);
				this->Run_(out, track);

				continue;
			}

			throw gcnew Exception();
		}
	}

	inline void Transcoder::InitFilter_(CodecContextWrapper& out)
	{
		const AVFilter* pBufferSource = nullptr;
		const AVFilter* pBufferSink = nullptr;

		InOutHandle outputs(avfilter_inout_alloc(), avfilter_inout_free);
		InOutHandle inputs(avfilter_inout_alloc(), avfilter_inout_free);
		AVFilterGraph* pFilterGraph = this->storage->filterGraph = avfilter_graph_alloc();

		if (!outputs.isValid() || !inputs.isValid() || pFilterGraph == nullptr)
			throw gcnew OutOfMemoryException();

		pBufferSource = avfilter_get_by_name("abuffer");
		pBufferSink = avfilter_get_by_name("abuffersink");

		if (pBufferSource == nullptr || pBufferSink == nullptr)
			throw gcnew AVException("Could not find \"abuffer\" and / or \"abuffersink\" filters.\n This should never happen.");

		AVCodecContext* pDecoderContext = this->dataIn->codecContext;

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


		if (FAILED(hr = avfilter_graph_create_filter(&this->storage->bufferSinkContext, pBufferSink, "out", nullptr, nullptr, pFilterGraph)))
			throw gcnew AVException(hr);

		AVFilterContext* pBufferSinkContext = this->storage->bufferSinkContext;

		hr = av_opt_set_bin(
			pBufferSinkContext,
			"sample_fmts",
			reinterpret_cast<unsigned char*>(&out.codecContext->sample_fmt),
			sizeof(out.codecContext->sample_fmt),
			AV_OPT_SEARCH_CHILDREN
		);
		if (FAILED(hr)) throw gcnew AVException(hr);

		hr = av_opt_set_bin(
			pBufferSinkContext,
			"channel_layouts",
			reinterpret_cast<unsigned char*>(&out.codecContext->channel_layout),
			sizeof(out.codecContext->channel_layout),
			AV_OPT_SEARCH_CHILDREN
		);
		if (FAILED(hr)) throw gcnew AVException(hr);

		hr = av_opt_set_bin(
			pBufferSinkContext,
			"sample_rates",
			reinterpret_cast<unsigned char*>(&out.codecContext->sample_rate),
			sizeof(out.codecContext->sample_rate),
			AV_OPT_SEARCH_CHILDREN
		);
		if (FAILED(hr)) throw gcnew AVException(hr);

		outputs->name = av_strdup("in");
		outputs->filter_ctx = this->storage->bufferSourceContext;
		outputs->pad_idx = 0;
		outputs->next = nullptr;

		inputs->name = av_strdup("out");
		inputs->filter_ctx = pBufferSinkContext;
		inputs->pad_idx = 0;
		inputs->next = nullptr;

		if (outputs->name == nullptr || inputs->name == nullptr)
			throw gcnew OutOfMemoryException();

		if (FAILED(hr = avfilter_graph_parse_ptr(pFilterGraph, "anull", &inputs, &outputs, nullptr)))
			throw gcnew AVException(hr);

		if (FAILED(hr = avfilter_graph_config(pFilterGraph, nullptr)))
			throw gcnew AVException(hr);

		av_buffersink_set_frame_size(pBufferSinkContext, out.codecContext->frame_size);
	}
	
	inline void Transcoder::Run_(CodecContextWrapper& out, ITrack^ track)
	{
		FrameHandle frame(av_frame_alloc(), av_frame_free);
		if (!frame.isValid()) throw gcnew OutOfMemoryException();

		FrameHandle filterFrame(av_frame_alloc(), av_frame_free);
		if (!filterFrame.isValid()) throw gcnew OutOfMemoryException();

		PacketHandle packet(av_packet_alloc(), av_packet_free);
		if (!packet.isValid()) throw gcnew OutOfMemoryException();

		PacketHandle encodedPacket(av_packet_alloc(), av_packet_free);
		if (!encodedPacket.isValid()) throw gcnew OutOfMemoryException();

		int start = track->Start.HasValue ? static_cast<int>(track->Start.Value.TotalSeconds) : 0;
		bool hasStop = track->Stop.HasValue;
		int stop = track->Stop.HasValue ? static_cast<int>(track->Stop.Value.TotalSeconds) : 0;

		logging("\"%s\" from %d, to %d\n", track->Title, start, stop);

		HRESULT tmp;
		while (SUCCEEDED(tmp = av_read_frame(this->dataIn->formatContext, packet)))
		{
			if (packet->stream_index == this->streamIndex)
			{
				double ts = packet->pts * av_q2d(this->dataIn->formatContext->streams[this->streamIndex]->time_base);
				if ((start == 0 || ts >= start) && (!hasStop || ts <= stop))
				{
					//logging("TS %f\n", ts);
					this->DecodePacket_(out, packet, frame, filterFrame, encodedPacket);
				}

				if (hasStop && ts > stop)
					break;
			}
			av_packet_unref(packet);
		}
		if (FAILED(tmp))
			logging("Run_: Error: %s\n", AVException::GetStringFromAVerror(tmp));

		this->FilterFrame_(out, nullptr, filterFrame, encodedPacket);

		this->EncodeWriteFrame_(out, nullptr, encodedPacket);

		HRESULT hr = S_OK;
		if (FAILED(hr = av_write_trailer(out.formatContext)))
			throw gcnew AVException(hr);
	}

	inline void Transcoder::DecodePacket_(CodecContextWrapper& out, AVPacket* pPacket, AVFrame* pFrame, AVFrame* pFilterFrame, AVPacket* pEncodedPacket)
	{

		av_packet_rescale_ts(
			pPacket,
			this->dataIn->formatContext->streams[this->streamIndex]->time_base,
			this->dataIn->codecContext->time_base
		);

		avcodec_send_packet(this->dataIn->codecContext, pPacket);

		HRESULT hr = S_OK;
		do
		{
			if (FAILED(hr = avcodec_receive_frame(this->dataIn->codecContext, pFrame)))
				break;

			pFrame->pts = pFrame->best_effort_timestamp;

			this->FilterFrame_(out, pFrame, pFilterFrame, pEncodedPacket);

			av_frame_unref(pFrame);
		} while (true);

		if (FAILED(hr) && hr != AVERROR(EAGAIN))
			logging("DecodePacket_: Error: %s\n", AVException::GetStringFromAVerror(hr));
	}

	inline void Transcoder::FilterFrame_(CodecContextWrapper& out, AVFrame* pFrame, AVFrame* pFilterFrame, AVPacket* pEncodedPacket)
	{
		av_buffersrc_add_frame_flags(this->storage->bufferSourceContext, pFrame, 0);

		HRESULT hr = S_OK;
		do
		{
			if (FAILED(hr = av_buffersink_get_frame(this->storage->bufferSinkContext, pFilterFrame)))
				break;

			pFilterFrame->pict_type = AV_PICTURE_TYPE_NONE;

			this->EncodeWriteFrame_(out, pFilterFrame, pEncodedPacket);

			av_frame_unref(pFilterFrame);
		} while (true);

		if (FAILED(hr) && hr != AVERROR(EAGAIN))
			logging("FilterFrame_: Error: %s\n", AVException::GetStringFromAVerror(hr));
	}

	inline void Transcoder::EncodeWriteFrame_(CodecContextWrapper& out, AVFrame* pFilterFrame, AVPacket* pEncodedPacket)
	{
		avcodec_send_frame(out.codecContext, pFilterFrame);

		HRESULT hr = S_OK;
		do
		{
			if (FAILED(hr = avcodec_receive_packet(out.codecContext, pEncodedPacket)))
				break;

			av_packet_rescale_ts(pEncodedPacket, out.codecContext->time_base, this->dataIn->codecContext->time_base);

			if (FAILED(hr = av_interleaved_write_frame(out.formatContext, pEncodedPacket)))
				break;

			av_packet_unref(pEncodedPacket);
		} while (true);

		if (FAILED(hr) && hr != AVERROR(EAGAIN))
			logging("EncodeWriteFrame_: Error: %s\n", AVException::GetStringFromAVerror(hr));

		av_packet_unref(pEncodedPacket);
	}
}