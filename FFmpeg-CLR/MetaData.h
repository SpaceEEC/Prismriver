#pragma once

#include "FormatContextWrapper.h"

using namespace System;
using namespace System::IO;
using namespace System::Collections::Generic;

namespace FFmpeg
{
	public ref class MetaData abstract sealed
	{
	public:
		/**
		 * Gets metadata from a stream.
		 */
		static Dictionary<String^, String^>^ Get(Stream^ stream);
		/**
		 * Gets metadata from a file.
		 */
		static Dictionary<String^, String^>^ Get(String^ file);

		/**
		 * Gets the thumbnail from a stream.
		 * Returns null if none was found.
		 */
		static Tuple<String^, array<unsigned char>^>^ GetThumbnail(Stream^ stream);
		/**
		 * Gets the thumbnail from a file.
		 * Returns null if none was found.
		 */
		static Tuple<String^, array<unsigned char>^>^ GetThumbnail(String^ file);
	private:
		static Dictionary<String^, String^>^ Get_(FormatContextWrapper% context);
		static Tuple<String^, array<unsigned char>^>^ GetThumbnail_(FormatContextWrapper% context);
	};
}