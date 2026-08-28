namespace Photoshop.NET;

/// <summary>
/// Describes an 8-bit RGB image buffer without an alpha channel.
/// </summary>
/// <param name="Pixels">The image bytes in row-major RGB order.</param>
/// <param name="Width">The image width in pixels.</param>
/// <param name="Height">The image height in pixels.</param>
/// <param name="StrideBytes">The row stride, or zero for a tightly packed image.</param>
public readonly record struct Rgb8Image(
    ReadOnlyMemory<byte> Pixels,
    uint Width,
    uint Height,
    uint StrideBytes = 0);
