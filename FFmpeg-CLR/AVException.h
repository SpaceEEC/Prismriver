#pragma once

#include <winerror.h>

extern "C"
{
#include <libavutil/error.h>
}

using namespace System;

namespace FFmpeg
{
	public ref class AVException : Exception
	{
	public:
		AVException(String^ message) : Exception(message, nullptr) {}
		AVException(String^ message, Exception^ e) : Exception(message, e) {}

		AVException(HRESULT hr) : AVException(hr, nullptr) {}
		AVException(HRESULT hr, Exception^ e) :
			Exception(AVException::GetStringFromAVerror(hr), e)
		{
			this->HResult = hr;
		}

		static String^ GetStringFromAVerror(HRESULT hr)
		{
			char errbuf[AV_ERROR_MAX_STRING_SIZE];
			av_make_error_string(reinterpret_cast<char*>(&errbuf), AV_ERROR_MAX_STRING_SIZE, hr);
			return gcnew String(errbuf);
		}
	};
}
