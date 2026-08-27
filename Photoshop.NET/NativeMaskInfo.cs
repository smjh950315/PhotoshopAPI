using System.Runtime.InteropServices;

namespace Photoshop.NET;

[StructLayout(LayoutKind.Sequential)]
internal struct NativeMaskInfo
{
    internal ulong Width;
    internal ulong Height;
    internal double CenterX;
    internal double CenterY;
    internal double Feather;
    internal byte HasMask;
    internal byte Disabled;
    internal byte RelativeToLayer;
    internal byte DefaultColor;
    internal byte HasDensity;
    internal byte Density;
    internal byte HasFeather;
    internal byte Reserved;
}
