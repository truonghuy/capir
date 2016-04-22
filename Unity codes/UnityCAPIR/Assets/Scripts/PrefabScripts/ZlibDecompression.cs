/*
 * Copyright (c) 2012 Truong-Huy D. Nguyen.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Public License v3.0
 * which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/gpl.html
 * 
 * Contributors:
 *     Truong-Huy D. Nguyen - initial API and implementation
 */

using System;
using System.IO;
using System.Text;
using Ionic.Zlib;

/// <summary>
/// Compression-decompression class. Used to inflate compressed Ftn files.
/// </summary>
/// <remarks>
/// Uses Zlib.
/// </remarks>
public class ZlibDecompression
{
	
	/// <summary>
	/// Decompresses the specified file.
	/// </summary>
	/// <param name='filename'>
	/// Filename.
	/// </param>
	/// <returns>
	/// The decompressed string.
	/// </returns>
	public static string decompress (string filename)
	{
		byte[] compressed = File.ReadAllBytes (filename);
		
		return ZlibCodecDecompress (compressed);
	}
	
	/// <summary>
	/// Compresses the specified string.
	/// </summary>
	/// <returns>
	/// The byte array of compressed text.
	/// </returns>
	/// <param name='textToCompress'>
	/// Text to compress.
	/// </param>
	private static byte[] ZlibCodecCompress (string textToCompress)
	{
		int outputSize = 524288; // 512K
		byte[] output = new Byte[ outputSize ];
		byte[] uncompressed = UTF8Encoding.UTF8.GetBytes (textToCompress);
		int lengthToCompress = uncompressed.Length;

		// If you want a ZLIB stream, set this to true.  If you want
		// a bare DEFLATE stream, set this to false.
		bool wantRfc1950Header = true;

		using (MemoryStream ms = new MemoryStream()) {
			ZlibCodec compressor = new ZlibCodec ();
			compressor.InitializeDeflate (CompressionLevel.BestCompression, wantRfc1950Header);  
        
			compressor.InputBuffer = uncompressed;
			compressor.AvailableBytesIn = lengthToCompress;
			compressor.NextIn = 0;
			compressor.OutputBuffer = output;

			foreach (var f in new FlushType[] { FlushType.None, FlushType.Finish }) {
				int bytesToWrite = 0;
				do {
					compressor.AvailableBytesOut = outputSize;
					compressor.NextOut = 0;
					compressor.Deflate (f);

					bytesToWrite = outputSize - compressor.AvailableBytesOut;
					if (bytesToWrite > 0)
						ms.Write (output, 0, bytesToWrite);
				} while (( f == FlushType.None && (compressor.AvailableBytesIn != 0 || compressor.AvailableBytesOut == 0)) ||
                       ( f == FlushType.Finish && bytesToWrite != 0));
			}

			compressor.EndDeflate ();

			ms.Flush ();
			return ms.ToArray ();
		}
	}
	
	/// <summary>
	/// Decompresses the specified byte array.
	/// </summary>
	/// <returns>
	/// The uncompressed text.
	/// </returns>
	/// <param name='compressed'>
	/// The compressed byte array to be decompressed.
	/// </param>
	private static string ZlibCodecDecompress (byte[] compressed)
	{
		int outputSize = 524288; // 512K
		byte[] output = new Byte[ outputSize ];
        
		// If you have a ZLIB stream, set this to true.  If you have
		// a bare DEFLATE stream, set this to false.
		bool expectRfc1950Header = true;
        
		using (MemoryStream ms = new MemoryStream()) {
			ZlibCodec compressor = new ZlibCodec ();
			compressor.InitializeInflate (expectRfc1950Header);
        
			compressor.InputBuffer = compressed;
			compressor.AvailableBytesIn = compressed.Length;
			compressor.NextIn = 0;
			compressor.OutputBuffer = output;

			foreach (var f in new FlushType[] { FlushType.None, FlushType.Finish }) {
				int bytesToWrite = 0;
				do {
					compressor.AvailableBytesOut = outputSize;
					compressor.NextOut = 0;
					compressor.Inflate (f);

					bytesToWrite = outputSize - compressor.AvailableBytesOut;
					if (bytesToWrite > 0)
						ms.Write (output, 0, bytesToWrite);
				} while (( f == FlushType.None && (compressor.AvailableBytesIn != 0 || compressor.AvailableBytesOut == 0)) ||
                       ( f == FlushType.Finish && bytesToWrite != 0));
			}

			compressor.EndInflate ();

			return UTF8Encoding.UTF8.GetString (ms.ToArray ());
		}
	}
}


