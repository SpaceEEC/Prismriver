#include "FFmpeg-CLR.h"

#include "BufferData.h"

constexpr int BUFFERSIZE = 16 * 1024;

namespace FFmpeg
{
	Dictionary<String^, String^>^ FFmpeg::GetMetaData(MemoryStream^ stream)
	{
		array<unsigned char>^ buffer = stream->GetBuffer();
		pin_ptr<unsigned char> pinned = &buffer[0];

		BufferData data(
			static_cast<unsigned char*>(pinned),
			buffer->LongLength
		);

		unsigned char* pFileStreamBuffer = static_cast<unsigned char*>(av_malloc(BUFFERSIZE));
		AVIOContext* pIOContext = avio_alloc_context(
			pFileStreamBuffer,
			BUFFERSIZE,
			0,
			&data,
			&data.ReadFunc,
			NULL,
			&data.SeekFunc
		);

		AVFormatContext* pFormatContext = avformat_alloc_context();

		pFormatContext->pb = pIOContext;
		pFormatContext->flags |= AVFMT_FLAG_CUSTOM_IO;

		try
		{
			if (FAILED(avformat_open_input(&pFormatContext, "", NULL, NULL)))
				throw gcnew Exception("Failed to open input stream.");
			
			Dictionary<String^, String^>^ dict = gcnew Dictionary<String^, String^>();

			AVDictionaryEntry* tag = NULL;
			while ((tag = av_dict_get(pFormatContext->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
				dict->Add(gcnew String(tag->key), gcnew String(tag->value));
			return dict;
		}
		finally
		{
			avformat_free_context(pFormatContext);
			av_freep(pIOContext);
		}
	}
}