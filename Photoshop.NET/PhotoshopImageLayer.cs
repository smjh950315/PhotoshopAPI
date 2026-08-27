namespace Photoshop.NET;

/// <summary>Writable raster image layer.</summary>
public sealed class PhotoshopImageLayer : PhotoshopLayer
{
    internal PhotoshopImageLayer(nint handle) : base(handle) { }

    internal static PhotoshopImageLayer CreateRgba8(
        ReadOnlySpan<byte> pixels,
        uint width,
        uint height,
        uint strideBytes,
        PhotoshopLayerOptions options)
    {
        PhotoshopApi.ValidateImageBuffer(pixels, width, height, strideBytes, 4, nameof(pixels));
        var copy = pixels.ToArray();
        using var nativeOptions = NativeLayerOptions.Create(options);
        using var nativePixels = new PinnedBuffer(copy);
        var view = new NativeRgba8View(nativePixels.Pointer, width, height, strideBytes);
        var optionsValue = nativeOptions.Value;
        var status = NativeMethods.CreateImageLayer(ref view, ref optionsValue, out var handle);
        PhotoshopApi.ThrowIfFailed(status);
        return new(handle);
    }
}
