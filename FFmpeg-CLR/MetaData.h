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

	private:
		static Dictionary<String^, String^>^ Get_(FormatContextWrapper% file);
	};
}