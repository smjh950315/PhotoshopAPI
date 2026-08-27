using System.Runtime.InteropServices;

namespace Photoshop.NET;

/// <summary>
/// Native-compatible view of an RGBA image buffer.
/// </summary>
[StructLayout(LayoutKind.Sequential)]
internal struct NativeRgba8View
{
    public NativeRgba8View(nint pixels, uint width, uint height, uint strideBytes)
    {
        Pixels = pixels;
        Width = width;
        Height = height;
        StrideBytes = strideBytes;
    }

    public nint Pixels;
    public uint Width;
    public uint Height;
    public uint StrideBytes;
}
