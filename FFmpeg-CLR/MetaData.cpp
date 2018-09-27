#include "MetaData.h"

namespace FFmpeg
{
#pragma region MetaData
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
#pragma endregion MetaData

#pragma region Thumbnail
	Tuple<String^, array<unsigned char>^>^ MetaData::GetThumbnail(String^ file)
	{
		return MetaData::GetThumbnail_(FormatContextWrapper(file));
	}
	Tuple<String^, array<unsigned char>^>^ MetaData::GetThumbnail(Stream^ stream)
	{
		return MetaData::GetThumbnail_(FormatContextWrapper(stream));
	}

	Tuple<String^, array<unsigned char>^>^ MetaData::GetThumbnail_(FormatContextWrapper% wrapper)
	{
		wrapper.openRead();

		int streamIndex = av_find_best_stream(wrapper.formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
		if (streamIndex == AVERROR_STREAM_NOT_FOUND) return nullptr;
		AVStream* stream = wrapper.formatContext->streams[streamIndex];

		if (stream->disposition != AV_DISPOSITION_ATTACHED_PIC) return nullptr;

		AVCodecID codecId = stream->codecpar->codec_id;
		String^ extension = codecId == AV_CODEC_ID_PNG
			? ".jpg"
			: codecId == AV_CODEC_ID_BMP
				? ".bmp"
				: ".jpeg";

		array<unsigned char>^ buffer = gcnew array<unsigned char>(stream->attached_pic.size);
		pin_ptr<unsigned char> pinned = &buffer[0];

		memcpy_s(
			pinned,
			stream->attached_pic.size,
			stream->attached_pic.data,
			stream->attached_pic.size
		);

		return Tuple::Create<String^, array<unsigned char>^>(extension, buffer);
	}
#pragma endregion Thumbnail
}