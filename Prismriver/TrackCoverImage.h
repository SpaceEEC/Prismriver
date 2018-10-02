#pragma once

#include "ImageFormat.h"

using namespace System;
using namespace System::Runtime::InteropServices;

namespace Prismriver
{
	public ref class TrackCoverImage
	{
	private:
		TrackCoverImage() { throw gcnew NotSupportedException(); }
		TrackCoverImage(TrackCoverImage% other) { throw gcnew NotSupportedException(); }
		TrackCoverImage% operator=(const TrackCoverImage% other) { throw gcnew NotSupportedException(); }
	internal:
		/**
		 * Allocates unmanaged memory, copies the bytes to it, and returns the pointer.
		 * The caller is responsible to free it.
		 */
		unsigned char* GetBytes()
		{
			unsigned char* bytes = static_cast<unsigned char*>(av_malloc(this->Bytes->Length));

			Marshal::Copy(
				this->Bytes,
				0,
				static_cast<IntPtr>(bytes),
				this->Bytes->Length
			);

			return bytes;
		}

		/**
		 * Returns the format casted to AVCodecID.
		 */
		AVCodecID GetCodecId()
		{
			return static_cast<AVCodecID>(this->Format);
		}

	public:
		/**
		 * Format of the cover image.
		 */
		initonly ImageFormat Format;

		/**
		 * Bytes representing the cover image.
		 */
		initonly array<unsigned char>^ Bytes;

		/**
		 * Instantiates a new TrackCoverImage specifying the format and bytes.
		 */
		TrackCoverImage(ImageFormat format, array<unsigned char>^ bytes) : Format(format), Bytes(bytes)
		{
			if (bytes == nullptr) throw gcnew ArgumentNullException("bytes");
			if (bytes->Length == 0) throw gcnew ArgumentException("Argument \"bytes\" may not be empty.");
		}
	};
}