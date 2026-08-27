namespace Photoshop.NET;

/// <summary>
/// Represents a Photoshop layer owned by the native library.
/// </summary>
public sealed class PhotoshopLayer : IDisposable
{
    private nint _handle;

    private PhotoshopLayer(nint handle)
    {
        _handle = handle;
    }

    /// <summary>
    /// Gets a value indicating whether the layer has been disposed.
    /// </summary>
    public bool IsDisposed => _handle == nint.Zero;

    /// <summary>
    /// Gets the most recent native error for this layer.
    /// </summary>
    public string LastError =>
        _handle == nint.Zero ? string.Empty : NativeMethods.GetLayerError(_handle);

    internal nint Handle => _handle;

    /// <summary>
    /// Creates an empty group layer.
    /// </summary>
    /// <param name="options">The initial layer properties.</param>
    /// <returns>A new group layer.</returns>
    public static PhotoshopLayer CreateGroup(PhotoshopLayerOptions options = default)
    {
        using var nativeOptions = NativeLayerOptions.Create(options);
        var optionsValue = nativeOptions.Value;
        var status = NativeMethods.CreateGroupLayer(ref optionsValue, out var handle);
        PhotoshopApi.ThrowIfFailed(status);
        return new PhotoshopLayer(handle);
    }

    /// <summary>
    /// Creates an image layer from an 8-bit RGBA buffer.
    /// </summary>
    /// <param name="pixels">The source RGBA bytes.</param>
    /// <param name="width">The image width in pixels.</param>
    /// <param name="height">The image height in pixels.</param>
    /// <param name="strideBytes">The row stride, or zero for a tightly packed image.</param>
    /// <param name="options">The initial layer properties.</param>
    /// <returns>A new image layer.</returns>
    public static PhotoshopLayer CreateImageRgba8(
        ReadOnlySpan<byte> pixels,
        uint width,
        uint height,
        uint strideBytes = 0,
        PhotoshopLayerOptions options = default)
    {
        PhotoshopApi.ValidateImageBuffer(pixels, width, height, strideBytes, 4, nameof(pixels));
        var copy = pixels.ToArray();
        using var nativeOptions = NativeLayerOptions.Create(options);
        using var nativePixels = new PinnedBuffer(copy);
        var view = new NativeRgba8View(nativePixels.Pointer, width, height, strideBytes);
        var optionsValue = nativeOptions.Value;
        var status = NativeMethods.CreateImageLayer(ref view, ref optionsValue, out var handle);
        PhotoshopApi.ThrowIfFailed(status);
        return new PhotoshopLayer(handle);
    }

    /// <summary>
    /// Creates an image layer from a described 8-bit RGBA image.
    /// </summary>
    /// <param name="image">The source image.</param>
    /// <param name="options">The initial layer properties.</param>
    /// <returns>A new image layer.</returns>
    public static PhotoshopLayer CreateImageRgba8(
        Rgba8Image image,
        PhotoshopLayerOptions options = default)
    {
        return CreateImageRgba8(
            image.Pixels.Span,
            image.Width,
            image.Height,
            image.StrideBytes,
            options);
    }

    /// <summary>
    /// Sets an 8-bit grayscale mask on the layer.
    /// </summary>
    /// <param name="pixels">The source mask bytes.</param>
    /// <param name="width">The mask width in pixels.</param>
    /// <param name="height">The mask height in pixels.</param>
    /// <param name="strideBytes">The row stride, or zero for a tightly packed mask.</param>
    public void SetMask8(ReadOnlySpan<byte> pixels, uint width, uint height, uint strideBytes = 0)
    {
        EnsureUsable();
        PhotoshopApi.ValidateImageBuffer(pixels, width, height, strideBytes, 1, nameof(pixels));
        var copy = pixels.ToArray();
        using var nativePixels = new PinnedBuffer(copy);
        var view = new NativeMask8View(nativePixels.Pointer, width, height, strideBytes);
        var status = NativeMethods.SetMask(_handle, ref view);
        PhotoshopApi.ThrowIfFailed(status, LastError);
    }

    /// <summary>
    /// Sets an 8-bit grayscale mask from a described image.
    /// </summary>
    /// <param name="image">The source mask image.</param>
    public void SetMask8(Mask8Image image)
    {
        SetMask8(image.Pixels.Span, image.Width, image.Height, image.StrideBytes);
    }

    /// <summary>
    /// Adds a child layer to this group layer.
    /// </summary>
    /// <param name="child">The child layer to add.</param>
    public void AddChild(PhotoshopLayer child)
    {
        ArgumentNullException.ThrowIfNull(child);
        EnsureUsable();
        child.EnsureUsable();

        var status = NativeMethods.GroupAddLayer(_handle, child.Handle);
        PhotoshopApi.ThrowIfFailed(status, LastError);
    }

    /// <summary>
    /// Releases the native layer handle.
    /// </summary>
    public void Dispose()
    {
        ReleaseHandle();
        GC.SuppressFinalize(this);
    }

    ~PhotoshopLayer()
    {
        ReleaseHandle();
    }

    internal void EnsureUsable()
    {
        ObjectDisposedException.ThrowIf(IsDisposed, this);
    }

    private void ReleaseHandle()
    {
        var handle = Interlocked.Exchange(ref _handle, nint.Zero);
        if (handle != nint.Zero)
        {
            NativeMethods.DestroyLayer(handle);
        }
    }
}
