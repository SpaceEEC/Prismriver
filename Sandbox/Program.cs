using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using FFmpeg;

namespace Sandbox
{
	class Program
	{
		static void Main(string[] args)
		{
			String fileName = @"test.mp4";
			String fileOut = @"output.raw";
			using (FFmpeg.FFmpeg ffmpeg = new FFmpeg.FFmpeg(fileName, fileOut))
				ffmpeg.DoStuff();

			Console.Read();
		}
	}
}
