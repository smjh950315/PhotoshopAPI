using System.Runtime.InteropServices;

namespace Photoshop.NET;

/// <summary>
/// Native-compatible layer option layout.
/// </summary>
[StructLayout(LayoutKind.Sequential)]
internal struct NativeLayerOptionsData
{
    public nint Name;
    public int Left;
    public int Top;
    public float Opacity;
    public byte Visible;
    public byte Locked;
    public byte Reserved0;
    public byte Reserved1;
}
