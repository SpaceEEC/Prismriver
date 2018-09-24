#include "FFmpeg-CLR.h"

#include "BufferData.h"

namespace FFmpeg
{
	Dictionary<String^, String^>^ FFmpeg::GetMetaData(Stream^ stream)
	{
		Buffer::Data data(stream);

		AVIOContext* pIOContext = avio_alloc_context(
			data.buffer,
			Buffer::BUFFERSIZE,
			0,
			data.stream,
			Buffer::ReadFunc,
			NULL,
			Buffer::SeekFunc
		);


		AVFormatContext* pFormatContext = avformat_alloc_context();

		pFormatContext->pb = pIOContext;

		try
		{
			if (FAILED(avformat_open_input(&pFormatContext, NULL, NULL, NULL)))
				throw gcnew Exception("Failed to open input stream.");

			Dictionary<String^, String^>^ dict = gcnew Dictionary<String^, String^>();

			AVDictionaryEntry* tag = NULL;
			while ((tag = av_dict_get(pFormatContext->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
				dict->Add(gcnew String(tag->key), gcnew String(tag->value));
			
			return dict;
		}
		finally
		{
		avformat_close_input(&pFormatContext);
		av_freep(&pIOContext->buffer);
		av_freep(&pIOContext);
		}
	}
}