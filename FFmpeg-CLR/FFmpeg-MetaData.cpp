#include "FFmpeg-CLR.h"

#include "FormatContextWrapper.h"

namespace FFmpeg
{
	Dictionary<String^, String^>^ FFmpeg::GetMetaData(Stream^ stream)
	{
		return FFmpeg::GetMetaData_(FormatContextWrapper(stream));
	}

	Dictionary<String^, String^>^ FFmpeg::GetMetaData(String^ file)
	{
		return FFmpeg::GetMetaData_(FormatContextWrapper(file));
	}

	Dictionary<String^, String^>^ FFmpeg::GetMetaData_(FormatContextWrapper% wrapper)
	{
		if (FAILED(wrapper.openRead()))
			throw gcnew Exception("Failed to open input stream.");

		Dictionary<String^, String^>^ dict = gcnew Dictionary<String^, String^>();

		AVDictionaryEntry* tag = NULL;
		while ((tag = av_dict_get(wrapper.formatContext->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
			dict->Add(gcnew String(tag->key), gcnew String(tag->value));

		return dict;
	}

}