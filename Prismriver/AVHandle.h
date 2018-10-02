#pragma once

extern "C"
{
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
}

namespace Prismriver
{
	// https://stackoverflow.com/a/16253197
	template<typename T, typename D>
	private class BaseHandle
	{
	private:
		BaseHandle() = delete;
		BaseHandle(const BaseHandle&) = delete;
		BaseHandle& operator=(const BaseHandle&) = delete;

	protected:
		T *val;
		D* deleter;

	public:
		BaseHandle(T *in, D* del) : val(in), deleter(del) {}

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
	};

	template<typename T, typename D>
	private class AVHandle : public BaseHandle<T, D>
	{
	public:
		AVHandle(T* in, D* del) : BaseHandle(in, del) {}

		~AVHandle()
		{
			deleter(&val);
		}
	};

	template<typename T>
	private class Handle : public BaseHandle<T, void(void*)>
	{
	public:
		Handle(T* in) : BaseHandle(in, nullptr) {}

		~Handle()
		{
			delete val;
		}
	};

	typedef AVHandle<AVFrame, void(AVFrame**)> FrameHandle;
	typedef AVHandle<AVPacket, void(AVPacket**)> PacketHandle;
	typedef AVHandle<AVFilterInOut, void(AVFilterInOut**)> InOutHandle;
	typedef AVHandle<AVFilterGraph, void(AVFilterGraph**)> FilterGraphHandle;
}
