using System.Runtime.InteropServices;

namespace Photoshop.NET;

[StructLayout(LayoutKind.Sequential)]
internal struct NativeDocumentInfo
{
    internal ulong Width;
    internal ulong Height;
    internal float Dpi;
    internal PhotoshopBitDepth BitDepth;
    internal PhotoshopColorMode ColorMode;
    internal uint RootLayerCount;
}
