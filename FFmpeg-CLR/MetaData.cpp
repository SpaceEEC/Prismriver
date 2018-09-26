#include "MetaData.h"

namespace FFmpeg
{
	Dictionary<String^, String^>^ MetaData::Get(Stream^ stream)
	{
		return MetaData::Get_(FormatContextWrapper(stream));
	}

	Dictionary<String^, String^>^ MetaData::Get(String^ file)
	{
		return MetaData::Get_(FormatContextWrapper(file));
	}

	Dictionary<String^, String^>^ MetaData::Get_(FormatContextWrapper% wrapper)
	{
		wrapper.openRead();

		Dictionary<String^, String^>^ dict = gcnew Dictionary<String^, String^>();

		AVDictionaryEntry* tag = nullptr;
		while ((tag = av_dict_get(wrapper.formatContext->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
			dict->Add(gcnew String(tag->key), gcnew String(tag->value));

		return dict;
	}
}