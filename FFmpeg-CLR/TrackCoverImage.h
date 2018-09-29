#pragma once

#include "ImageFormat.h"

using namespace System;

namespace FFmpeg
{
	public ref class TrackCoverImage
	{
	private:
		TrackCoverImage() { throw gcnew NotSupportedException(); }
		TrackCoverImage(TrackCoverImage% other) { throw gcnew NotSupportedException(); }
		TrackCoverImage% operator=(const TrackCoverImage% other) { throw gcnew NotSupportedException(); }
	public:
		/**
		 * Bytes representing the cover image.
		 */
		initonly array<unsigned char>^ Bytes;
		/**
		 * Format of the cover image.
		 */
		initonly ImageFormat Format;

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