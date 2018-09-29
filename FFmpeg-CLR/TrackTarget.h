#pragma once

using namespace System;

namespace FFmpeg
{
	public ref struct TrackTarget
	{
	private:
		TrackTarget() { throw gcnew NotSupportedException(); }
		TrackTarget(TrackTarget% other) { throw gcnew NotSupportedException(); }
		TrackTarget% operator=(const TrackTarget% other) { throw gcnew NotSupportedException(); }

	public:
		/**
		 * The stream this TrackTarget was isntantiated, or null if none.
		 */
		initonly System::IO::Stream^ Stream;
		/**
		 * The file this TrackTarget was isntantiated with, or null if none.
		 */
		initonly String^ File;
		/**
		 * The stream this TrackTarget was isntantiated, or null if none.
		 */
		initonly String^ Format;

		/**
		 * Instantiates a new TrackTarget with a stream and a format.
		 * Will guess the output format based on the passed format.
		 */
		TrackTarget(System::IO::Stream^ stream, String^ format) : Stream(stream), File(nullptr), Format(format)
		{
			if (stream == nullptr) throw gcnew ArgumentNullException("stream");
			if (format == nullptr) throw gcnew ArgumentNullException("format");
		}

		/**
		 * Instantiates a new TrackTarget with a file name.
		 * Will guess the output format based on file extension.
		 */
		TrackTarget(String^ file) : TrackTarget(file, nullptr) {}
		/**
		 * Instantiates a new TrackTarget with a file name and format.
		 * Will guess the output format based on the format, or the file name if format is null.
		 */
		TrackTarget(String^ file, String^ format) : Stream(nullptr), File(file), Format(format)
		{
			if (file == nullptr) throw gcnew ArgumentNullException("file");
		}
	};
}