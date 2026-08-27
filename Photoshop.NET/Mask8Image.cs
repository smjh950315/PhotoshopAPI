namespace Photoshop.NET;

/// <summary>
/// Describes an 8-bit grayscale layer mask buffer.
/// </summary>
/// <param name="Pixels">The mask bytes in row-major order.</param>
/// <param name="Width">The mask width in pixels.</param>
/// <param name="Height">The mask height in pixels.</param>
/// <param name="StrideBytes">The row stride, or zero for a tightly packed mask.</param>
public readonly record struct Mask8Image(
    ReadOnlyMemory<byte> Pixels,
    uint Width,
    uint Height,
    uint StrideBytes = 0);
