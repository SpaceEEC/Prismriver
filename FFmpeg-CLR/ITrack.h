#pragma once

using namespace System;

namespace FFmpeg
{
	public interface class ITrack
	{
		/**
		 * Image of this ITrack, null if none.
		 */
		property array<unsigned char>^ CoverImage
		{
			array<unsigned char>^ get();
		}

		/**
		 * Where this ITrack starts, null if at the start.
		 */
		property Nullable<TimeSpan> Start
		{
			Nullable<TimeSpan> get();
		}

		/**
		 * Where this ITrack stops, null if at the end.
		 */
		property Nullable<TimeSpan> Stop
		{
			Nullable<TimeSpan> get();
		}

		/**
		 * Title of this ITrack, null if none.
		 */
		property String^ Title
		{
			String^ get();
		}

		/**
		 * Author of this ITrack, null if none.
		 */
		property String^ Author
		{
			String^ get();
		}

		/**
		 * Album of this ITrack, null if none.
		 */
		property String^ Album
		{
			String^ get();
		}

		/**
		 * Where to output this ITrack.
		 * This either has to be a string, will be interpreted as the filepath then.
		 * Or a System.IO.Stream, which will be written to then.
		 * Should be seekable as headers sometimes needs to be rewritten.
		 *
		 * May _not_ be null.
		 */
		property Object^ Target
		{
			Object^ get();
		}
	};
}
