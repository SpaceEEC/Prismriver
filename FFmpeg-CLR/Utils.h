#pragma once

#include <string.h>

using namespace System;
using namespace System::Text;

namespace FFmpeg
{
	private ref class Utils sealed abstract
	{
	internal:
		static String^ utf8toString(char* bytes)
		{
			return Encoding::UTF8->GetString(
				reinterpret_cast<unsigned char*>(bytes),
				static_cast<int>(strlen(bytes))
			);
		}

		static char* stringtoUtf8(String^ string)
		{
			array<unsigned char>^ bytes = Encoding::UTF8->GetBytes(string + "\0");

			char* chars = static_cast<char*>(malloc(bytes->Length));

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
	};
}
