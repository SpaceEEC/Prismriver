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

namespace FFmpeg
{
#define logging(fmt, ...) av_log(nullptr, AV_LOG_INFO, fmt, __VA_ARGS__)

	Transcoder::Transcoder(Stream^ stream, array<ITrack^>^ tracks)
		: dataIn(new CodecContextWrapper(stream)),
		Tracks(tracks) {}

	Transcoder::Transcoder(String^ file, array<ITrack^>^ tracks)
		: dataIn(new CodecContextWrapper(file)),
		Tracks(tracks) {}

	Transcoder::~Transcoder() { this->!Transcoder(); }
	Transcoder::!Transcoder()
	{
		GC::SuppressFinalize(this);
		delete this->storage;
		delete this->dataIn;
	}

	Transcoder^ Transcoder::SetProgress(IProgress<Tuple<int, double>^>^ progress)
	{
		this->progress_ = progress;

		return this;
	}

	void Transcoder::Run()
	{
		this->trackIndex_ = -1;
		this->dataIn->openRead();
		this->storage = new Storage();

		try
		{
			ITrack^ prev = nullptr;
			for each(ITrack^ track in this->Tracks)
			{
				++this->trackIndex_;
				if (prev != nullptr && !track->Start.HasValue)
					track->Start = prev->Stop;
				prev = track;

				if (this->dataOut_ != nullptr) delete this->dataOut_;
				this->dataOut_ = track->Target->Stream != nullptr
					? new CodecContextWrapper(track->Target->Stream)
					: new CodecContextWrapper(track->Target->File);

				this->dataOut_->openWrite(this->dataIn);

				AVDictionary* meta = this->dataOut_->formatContext->metadata;

				HRESULT hr = av_dict_copy(&meta, dataIn->formatContext->metadata, AV_DICT_IGNORE_SUFFIX);
				if (FAILED(hr))
					throw gcnew AVException(hr);

				if (track->Album != nullptr) Utils::AddStringToDict(&meta, "album", track->Album);
				if (track->Author != nullptr) Utils::AddStringToDict(&meta, "artist", track->Author);
				if (track->Title != nullptr) Utils::AddStringToDict(&meta, "title", track->Title);

				this->dataOut_->formatContext->metadata = meta;
				av_dict_copy(&this->dataOut_->formatContext->streams[0]->metadata, meta, AV_DICT_IGNORE_SUFFIX);

				hr = avformat_write_header(this->dataOut_->formatContext, nullptr);
				if (FAILED(hr))
					throw gcnew AVException(hr);

				this->InitFilter_();
				this->Run_();
			}
		}
		finally
		{
			if (this->dataOut_ != nullptr) delete this->dataOut_;
		}
	}

	inline void Transcoder::InitFilter_()
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
		AVCodecContext* pEncCtx = this->dataOut_->codecContext;

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

		av_buffersink_set_frame_size(pBufferSinkContext, pEncCtx->frame_size);
	}

	inline void Transcoder::Run_()
	{
		FrameHandle frame(av_frame_alloc(), av_frame_free);
		if (!frame.isValid()) throw gcnew OutOfMemoryException();

		FrameHandle filterFrame(av_frame_alloc(), av_frame_free);
		if (!filterFrame.isValid()) throw gcnew OutOfMemoryException();

		PacketHandle packet(av_packet_alloc(), av_packet_free);
		if (!packet.isValid()) throw gcnew OutOfMemoryException();

		PacketHandle encodedPacket(av_packet_alloc(), av_packet_free);
		if (!encodedPacket.isValid()) throw gcnew OutOfMemoryException();

		ITrack^ track = this->Tracks[this->trackIndex_];
		int start = track->Start.HasValue ? static_cast<int>(track->Start.Value.TotalSeconds) : 0;
		bool hasStop = track->Stop.HasValue;
		int stop = track->Stop.HasValue ? static_cast<int>(track->Stop.Value.TotalSeconds) : 0;

		logging("\"%s\" from %d, to %d\n", track->Title, start, stop);

		HRESULT tmp;
		while (SUCCEEDED(tmp = av_read_frame(this->dataIn->formatContext, packet)))
		{
			if (packet->stream_index == this->dataIn->streamIndex)
			{
				// rational of the current position in the stream in seconds
				AVRational ts = av_mul_q(this->dataIn->getStream()->time_base, { static_cast<int>(packet->pts), 1 });
				// doulbe of ^
				double dts = av_q2d(ts);
				// Report back
				this->ReportProgress_(dts);
				if ((dts >= start) && (!hasStop || dts <= stop))
				{
					// subtract the start of the current track
					ts = av_sub_q(ts, { start, 1 });
					// convert back to to time_base of the input stream
					ts = av_div_q(ts, this->dataIn->getStream()->time_base);
					// to double and trucante
					packet->pts = static_cast<long long>(av_q2d(ts));
					this->DecodePacket_(packet, frame, filterFrame, encodedPacket);
				}

				if (hasStop && dts > stop)
				{
					av_packet_unref(packet);
					break;
				}
			}
			av_packet_unref(packet);
		}
		if (FAILED(tmp))
			logging("Run_: Error: %s\n", AVException::GetStringFromAVerror(tmp));

		this->FilterFrame_(nullptr, filterFrame, encodedPacket);
		this->EncodeWriteFrame_(nullptr, encodedPacket);

		HRESULT hr = S_OK;
		if (FAILED(hr = av_write_trailer(this->dataOut_->formatContext)))
			throw gcnew AVException(hr);
	}

	inline void Transcoder::DecodePacket_(AVPacket* pPacket, AVFrame* pFrame, AVFrame* pFilterFrame, AVPacket* pEncodedPacket)
	{
		av_packet_rescale_ts(
			pPacket,
			this->dataIn->getStream()->time_base,
			this->dataIn->codecContext->time_base
		);

		avcodec_send_packet(this->dataIn->codecContext, pPacket);

		HRESULT hr = S_OK;
		do
		{
			if (FAILED(hr = avcodec_receive_frame(this->dataIn->codecContext, pFrame)))
				break;

			this->FilterFrame_(pFrame, pFilterFrame, pEncodedPacket);

			av_frame_unref(pFrame);
		} while (true);

		if (FAILED(hr) && hr != AVERROR(EAGAIN))
			logging("DecodePacket_: Error: %s\n", AVException::GetStringFromAVerror(hr));
	}

	inline void Transcoder::FilterFrame_(AVFrame* pFrame, AVFrame* pFilterFrame, AVPacket* pEncodedPacket)
	{
		av_buffersrc_add_frame_flags(this->storage->bufferSourceContext, pFrame, 0);

		HRESULT hr = S_OK;
		do
		{
			if (FAILED(hr = av_buffersink_get_frame(this->storage->bufferSinkContext, pFilterFrame)))
				break;

			pFilterFrame->pict_type = AV_PICTURE_TYPE_NONE;

			this->EncodeWriteFrame_(pFilterFrame, pEncodedPacket);

			av_frame_unref(pFilterFrame);
		} while (true);

		if (FAILED(hr) && hr != AVERROR(EAGAIN))
			logging("FilterFrame_: Error: %s\n", AVException::GetStringFromAVerror(hr));
	}

	inline void Transcoder::EncodeWriteFrame_(AVFrame* pFilterFrame, AVPacket* pEncodedPacket)
	{
		avcodec_send_frame(this->dataOut_->codecContext, pFilterFrame);

		HRESULT hr = S_OK;
		do
		{
			if (FAILED(hr = avcodec_receive_packet(this->dataOut_->codecContext, pEncodedPacket)))
				break;

			av_packet_rescale_ts(pEncodedPacket, this->dataOut_->codecContext->time_base, this->dataIn->codecContext->time_base);

			if (FAILED(hr = av_interleaved_write_frame(this->dataOut_->formatContext, pEncodedPacket)))
				break;

			av_packet_unref(pEncodedPacket);
		} while (true);

		if (FAILED(hr) && hr != AVERROR(EAGAIN))
			logging("EncodeWriteFrame_: Error: %s\n", AVException::GetStringFromAVerror(hr));

		av_packet_unref(pEncodedPacket);
	}

	void Transcoder::ReportProgress_(double current)
	{
		if (this->progress_ == nullptr) return;
		ITrack^ track = this->Tracks[this->trackIndex_];

		double stop;
		// The track provides a stop value, blindly trust it
		if (track->Stop.HasValue)
			stop = track->Stop.Value.TotalSeconds;
		// Can't get a duration, report NaN
		else if (this->dataIn->getStream()->duration < 0)
			stop = NAN;
		// End of stream
		else
		{
			// get seconds in AVRational
			AVRational rat = av_mul_q({ static_cast<int>(this->dataIn->getStream()->duration), 1 }, this->dataIn->getStream()->time_base);
			// convert to double
			stop = av_q2d(rat);
		}

		// If we are not at the start, subtract the offset
		if (track->Start.HasValue)
		{
			current -= track->Start.Value.TotalSeconds;
			stop -= track->Start.Value.TotalSeconds;
		}

		double progress = current / stop;
		// never report more than 1
		if (progress > 1) progress = 1;

		this->progress_->Report(
			gcnew Tuple<int, double>(
				this->trackIndex_,
				progress
				)
		);
	}
}
