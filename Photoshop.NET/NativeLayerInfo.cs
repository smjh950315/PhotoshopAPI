using System.Runtime.InteropServices;

namespace Photoshop.NET;

[StructLayout(LayoutKind.Sequential)]
internal struct NativeLayerInfo
{
    internal PhotoshopLayerType Type;
    internal PhotoshopBitDepth BitDepth;
    internal PhotoshopColorMode ColorMode;
    internal PhotoshopBlendMode BlendMode;
    internal PhotoshopLayerColor DisplayColor;
    internal uint Width;
    internal uint Height;
    internal float CenterX;
    internal float CenterY;
    internal float Opacity;
    internal float Fill;
    internal byte Visible;
    internal byte Locked;
    internal byte ClippingMask;
    internal byte Reserved;
}
