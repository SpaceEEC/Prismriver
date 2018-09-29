#pragma once

extern "C"
{
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
}

namespace FFmpeg
{
	// https://stackoverflow.com/a/16253197
	template<typename T, typename D>
	private class AVHandle
	{
	private:
		AVHandle() = delete;
		AVHandle(const AVHandle&) = delete;
		AVHandle& operator=(const AVHandle&) = delete;

		T *val;
		D* deleter;

	public:
		AVHandle(T *in, D* del) : val(in), deleter(del) {}

		operator T *()
		{
			return val;
		}

		T* operator->()
		{
			return val;
		}

		T** operator&()
		{
			return &val;
		}

		bool isValid()
		{
			return val != nullptr;
		}

		~AVHandle()
		{
			deleter(&val);
		}
	};

	typedef AVHandle<AVFrame, void(AVFrame**)> FrameHandle;
	typedef AVHandle<AVPacket, void(AVPacket**)> PacketHandle;
	typedef AVHandle<AVFilterInOut, void(AVFilterInOut**)> InOutHandle;
	typedef AVHandle<AVFilterGraph, void(AVFilterGraph**)> FilterGraphHandle;
}
