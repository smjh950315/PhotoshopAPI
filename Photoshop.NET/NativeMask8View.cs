using System.Runtime.InteropServices;

namespace Photoshop.NET;

/// <summary>
/// Native-compatible view of an 8-bit layer mask.
/// </summary>
[StructLayout(LayoutKind.Sequential)]
internal struct NativeMask8View
{
    public NativeMask8View(nint pixels, uint width, uint height, uint strideBytes)
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
