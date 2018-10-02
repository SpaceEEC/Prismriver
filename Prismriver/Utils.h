#pragma once

#include <string.h>

using namespace System;
using namespace System::Text;

namespace Prismriver
{
	private ref class Utils sealed abstract
	{
	internal:
		static inline String^ Utf8BytesToString(char* bytes)
		{
			return Encoding::UTF8->GetString(
				reinterpret_cast<unsigned char*>(bytes),
				static_cast<int>(strlen(bytes))
			);
		}

		static inline char* StringToUtf8Bytes(String^ string)
		{
			array<unsigned char>^ bytes = Encoding::UTF8->GetBytes(string + "\0");

			char* chars = static_cast<char*>(av_malloc(bytes->Length));

			{
				pin_ptr<unsigned char> pinned = &bytes[0];
				memcpy_s(
					chars,
					bytes->Length,
					pinned,
					bytes->Length
				);
			}

			delete bytes;

			return chars;
		}

		static inline void AddStringToDict(AVDictionary** dict, String^ key, String^ value)
		{
			char* pKey = Utils::StringToUtf8Bytes(key);
			char* pValue = Utils::StringToUtf8Bytes(value);

			av_dict_set(
				dict,
				pKey,
				pValue,
				AV_DICT_DONT_STRDUP_KEY | AV_DICT_DONT_STRDUP_VAL
			);
		}
	};
}
