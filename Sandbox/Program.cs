using Prismriver;
using System;

namespace Sandbox
{
	class Program : IProgress<Tuple<Int32, Double>>
	{
		private class Track : ITrack
		{
			public string Album => "TestAlbum";

			public string Author { get; set; }

			public string Title { get; set; }
			public TimeSpan? Stop { get; set; } = null;

			public TimeSpan? Start { get; set; } = null;

			public TrackTarget Target { get; set; }

			public TrackCoverImage CoverImage => null;
		}

		public static void Main(string[] args) => new Program().Run();

		private Double _last = Double.NegativeInfinity;
		public void Report(Tuple<Int32, Double> value)
		{
			Double current = value.Item2 * 100;
			if (Math.Truncate(this._last) == Math.Truncate(current))
			{
				this._last = current;
				return;
			}

			this._last = current;

			Console.WriteLine($"Transcoding Track {value.Item1 + 1}: {Math.Round(current, 2)}%");
		}
		public void Run()
		{
			ITrack[] tracks = new ITrack[]
			{
				/**/
				new Track()
				{
					Author = "Author1",
					Title = "Title1",
					Stop = TimeSpan.FromSeconds(60 + 52),
					Target = new TrackTarget(@".\out\title1.flac")
				},
				new Track()
				{
					Author = "Author2",
					Title = "Title2",
					Stop = TimeSpan.FromSeconds(7 * 6 + 5),
					Target = new TrackTarget(@".\out\title2.flac")
				}
				/**/
			};

			using (Transcoder transcoder = new Transcoder("tmp", tracks))
				transcoder.SetProgress(this).Run();

			Console.Read();
		}
	}
}
