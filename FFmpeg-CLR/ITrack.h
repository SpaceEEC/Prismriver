#pragma once

#include "TrackCoverImage.h"
#include "TrackTarget.h"

using namespace System;

namespace FFmpeg
{
	public interface class ITrack
	{
		/**
		 * Image of this ITrack, null if none.
		 */
		property TrackCoverImage^ CoverImage
		{
			TrackCoverImage^ get();
		}

		/**
		 * Where this ITrack starts, null if directly after the preceding track or at the start.
		 * In case of null this field will be automatically filled with the stop of the preceeding track.
		 */
		property Nullable<TimeSpan> Start
		{
			Nullable<TimeSpan> get();
			void set(Nullable<TimeSpan> value);
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
		 */
		property TrackTarget^ Target
		{
			TrackTarget^ get();
		}
	};
}
