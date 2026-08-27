using System.Runtime.InteropServices;

namespace Photoshop.NET;

/// <summary>Base wrapper for common Photoshop layer state and pixel/mask access.</summary>
public class PhotoshopLayer : IDisposable
{
    private nint _handle;

    internal PhotoshopLayer(nint handle) => _handle = handle;

    /// <summary>Gets whether this wrapper has released its native handle.</summary>
    public bool IsDisposed => _handle == nint.Zero;
    /// <summary>Gets the latest detailed native error.</summary>
    public string LastError => IsDisposed ? string.Empty : NativeMethods.GetLayerError(_handle);
    internal nint Handle => _handle;

    /// <summary>Gets or replaces the common editable layer properties.</summary>
    public PhotoshopLayerInfo Info
    {
        get
        {
            EnsureUsable();
            var status = NativeMethods.GetLayerInfo(_handle, out var value);
            Throw(status);
            return FromNative(value);
        }
        set
        {
            EnsureUsable();
            var native = ToNative(value);
            Throw(NativeMethods.SetLayerInfo(_handle, ref native));
        }
    }

    /// <summary>Gets or sets the UTF-8 layer name.</summary>
    public string Name
    {
        get
        {
            EnsureUsable();
            return NativeMethods.ReadString((nint b, uint c, out uint r) => NativeMethods.GetLayerName(_handle, b, c, out r));
        }
        set
        {
            ArgumentNullException.ThrowIfNull(value);
            EnsureUsable();
            using var utf8 = new Utf8Buffer(value);
            Throw(NativeMethods.SetLayerName(_handle, utf8.Pointer));
        }
    }

    /// <summary>Creates an empty 8-bit group layer.</summary>
    public static PhotoshopGroupLayer CreateGroup(PhotoshopLayerOptions options = default) => PhotoshopGroupLayer.Create(options);

    /// <summary>Creates an 8-bit image layer from straight-alpha RGBA pixels.</summary>
    public static PhotoshopImageLayer CreateImageRgba8(ReadOnlySpan<byte> pixels, uint width, uint height, uint strideBytes = 0, PhotoshopLayerOptions options = default) =>
        PhotoshopImageLayer.CreateRgba8(pixels, width, height, strideBytes, options);

    /// <summary>Creates an 8-bit image layer from a described RGBA image.</summary>
    public static PhotoshopImageLayer CreateImageRgba8(Rgba8Image image, PhotoshopLayerOptions options = default) =>
        PhotoshopImageLayer.CreateRgba8(image.Pixels.Span, image.Width, image.Height, image.StrideBytes, options);

    /// <summary>Gets available Photoshop channel indices, including mask index -2.</summary>
    public IReadOnlyList<int> GetChannelIndices()
    {
        EnsureUsable();
        var status = NativeMethods.GetChannelIndices(_handle, nint.Zero, 0, out var count);
        Throw(status);
        if (count == 0) return [];
        var values = new int[checked((int)count)];
        var bytes = MemoryMarshal.AsBytes(values.AsSpan()).ToArray();
        using var pinned = new PinnedBuffer(bytes);
        status = NativeMethods.GetChannelIndices(_handle, pinned.Pointer, count, out var actual);
        Throw(status);
        MemoryMarshal.Cast<byte, int>(bytes).CopyTo(values);
        if (actual != count) Array.Resize(ref values, checked((int)actual));
        return values;
    }

    /// <summary>Gets one channel as its native sample bytes.</summary>
    public byte[] GetChannelBytes(int channelIndex)
    {
        EnsureUsable();
        return NativeMethods.ReadBytes((nint b, ulong c, out ulong r) => NativeMethods.GetChannelData(_handle, channelIndex, b, c, out r));
    }

    /// <summary>Gets one channel as 8-bit samples.</summary>
    public byte[] GetChannel8(int channelIndex) { RequireDepth(PhotoshopBitDepth.Bit8); return GetChannelBytes(channelIndex); }
    /// <summary>Gets one channel as 16-bit samples.</summary>
    public ushort[] GetChannel16(int channelIndex) { RequireDepth(PhotoshopBitDepth.Bit16); return MemoryMarshal.Cast<byte, ushort>(GetChannelBytes(channelIndex)).ToArray(); }
    /// <summary>Gets one channel as 32-bit floating-point samples.</summary>
    public float[] GetChannel32(int channelIndex) { RequireDepth(PhotoshopBitDepth.Bit32); return MemoryMarshal.Cast<byte, float>(GetChannelBytes(channelIndex)).ToArray(); }

    /// <summary>Replaces one writable channel using native sample bytes.</summary>
    public void SetChannelBytes(int channelIndex, ReadOnlySpan<byte> samples)
    {
        EnsureUsable();
        var copy = samples.ToArray();
        if (copy.Length == 0) throw new ArgumentException("Channel data must not be empty.", nameof(samples));
        using var pinned = new PinnedBuffer(copy);
        Throw(NativeMethods.SetChannelData(_handle, channelIndex, pinned.Pointer, (ulong)copy.Length));
    }

    /// <summary>Replaces one 8-bit channel.</summary>
    public void SetChannel8(int channelIndex, ReadOnlySpan<byte> samples) { RequireDepth(PhotoshopBitDepth.Bit8); SetChannelBytes(channelIndex, samples); }
    /// <summary>Replaces one 16-bit channel.</summary>
    public void SetChannel16(int channelIndex, ReadOnlySpan<ushort> samples) { RequireDepth(PhotoshopBitDepth.Bit16); SetChannelBytes(channelIndex, MemoryMarshal.AsBytes(samples)); }
    /// <summary>Replaces one 32-bit floating-point channel.</summary>
    public void SetChannel32(int channelIndex, ReadOnlySpan<float> samples) { RequireDepth(PhotoshopBitDepth.Bit32); SetChannelBytes(channelIndex, MemoryMarshal.AsBytes(samples)); }

    /// <summary>Gets current layer-mask metadata.</summary>
    public PhotoshopMaskInfo GetMaskInfo()
    {
        EnsureUsable();
        var status = NativeMethods.GetMaskInfo(_handle, out var value);
        Throw(status);
        return new(value.Width, value.Height, value.CenterX, value.CenterY,
            value.HasFeather != 0 ? value.Feather : null, value.HasMask != 0,
            value.Disabled != 0, value.RelativeToLayer != 0, value.DefaultColor,
            value.HasDensity != 0 ? value.Density : null);
    }

    /// <summary>Updates mask position and flags; mask pixel dimensions remain unchanged.</summary>
    public void SetMaskInfo(PhotoshopMaskInfo value)
    {
        EnsureUsable();
        var native = new NativeMaskInfo
        {
            Width = value.Width, Height = value.Height, CenterX = value.CenterX, CenterY = value.CenterY,
            Feather = value.Feather ?? 0, HasMask = value.HasMask ? (byte)1 : (byte)0,
            Disabled = value.Disabled ? (byte)1 : (byte)0, RelativeToLayer = value.RelativeToLayer ? (byte)1 : (byte)0,
            DefaultColor = value.DefaultColor, HasDensity = value.Density.HasValue ? (byte)1 : (byte)0,
            Density = value.Density ?? 0, HasFeather = value.Feather.HasValue ? (byte)1 : (byte)0
        };
        Throw(NativeMethods.SetMaskInfo(_handle, ref native));
    }

    /// <summary>Gets mask pixels as native sample bytes.</summary>
    public byte[] GetMaskBytes()
    {
        EnsureUsable();
        return NativeMethods.ReadBytes((nint b, ulong c, out ulong r) => NativeMethods.GetMaskData(_handle, b, c, out r));
    }

    /// <summary>Replaces mask pixels using the document's native sample representation.</summary>
    public void SetMaskBytes(ReadOnlySpan<byte> samples, ulong width, ulong height)
    {
        EnsureUsable();
        var copy = samples.ToArray();
        if (copy.Length == 0) throw new ArgumentException("Mask data must not be empty.", nameof(samples));
        using var pinned = new PinnedBuffer(copy);
        Throw(NativeMethods.SetMaskData(_handle, pinned.Pointer, (ulong)copy.Length, width, height));
    }

    /// <summary>Sets a strided 8-bit grayscale mask.</summary>
    public void SetMask8(ReadOnlySpan<byte> pixels, uint width, uint height, uint strideBytes = 0)
    {
        EnsureUsable();
        RequireDepth(PhotoshopBitDepth.Bit8);
        PhotoshopApi.ValidateImageBuffer(pixels, width, height, strideBytes, 1, nameof(pixels));
        var copy = pixels.ToArray();
        using var pinned = new PinnedBuffer(copy);
        var view = new NativeMask8View(pinned.Pointer, width, height, strideBytes);
        Throw(NativeMethods.SetMask8(_handle, ref view));
    }

    /// <summary>Sets a described 8-bit grayscale mask.</summary>
    public void SetMask8(Mask8Image image) => SetMask8(image.Pixels.Span, image.Width, image.Height, image.StrideBytes);

    /// <inheritdoc />
    public void Dispose() { ReleaseHandle(); GC.SuppressFinalize(this); }
    ~PhotoshopLayer() => ReleaseHandle();

    internal static PhotoshopLayer FromHandle(nint handle)
    {
        var status = NativeMethods.GetLayerInfo(handle, out var info);
        if (status != PhotoshopStatus.Success)
        {
            var error = NativeMethods.GetLayerError(handle);
            NativeMethods.DestroyLayer(handle);
            PhotoshopApi.ThrowIfFailed(status, error);
        }
        return info.Type switch
        {
            PhotoshopLayerType.Group => new PhotoshopGroupLayer(handle),
            PhotoshopLayerType.Image => new PhotoshopImageLayer(handle),
            PhotoshopLayerType.Text => new PhotoshopTextLayer(handle),
            PhotoshopLayerType.SmartObject => new PhotoshopSmartObjectLayer(handle),
            _ => new PhotoshopLayer(handle)
        };
    }

    internal void EnsureUsable() => ObjectDisposedException.ThrowIf(IsDisposed, this);
    internal void Throw(PhotoshopStatus status) => PhotoshopApi.ThrowIfFailed(status, LastError);

    private void RequireDepth(PhotoshopBitDepth expected)
    {
        if (Info.BitDepth != expected) throw new InvalidOperationException($"Layer uses {Info.BitDepth}, not {expected} samples.");
    }

    private static PhotoshopLayerInfo FromNative(NativeLayerInfo value) => new(
        value.Type, value.BitDepth, value.ColorMode, value.BlendMode, value.DisplayColor,
        value.Width, value.Height, value.CenterX, value.CenterY, value.Opacity, value.Fill,
        value.Visible != 0, value.Locked != 0, value.ClippingMask != 0);

    private static NativeLayerInfo ToNative(PhotoshopLayerInfo value) => new()
    {
        Type = value.Type, BitDepth = value.BitDepth, ColorMode = value.ColorMode,
        BlendMode = value.BlendMode, DisplayColor = value.DisplayColor, Width = value.Width,
        Height = value.Height, CenterX = value.CenterX, CenterY = value.CenterY,
        Opacity = value.Opacity, Fill = value.Fill, Visible = value.Visible ? (byte)1 : (byte)0,
        Locked = value.Locked ? (byte)1 : (byte)0, ClippingMask = value.ClippingMask ? (byte)1 : (byte)0
    };

    private void ReleaseHandle()
    {
        var handle = Interlocked.Exchange(ref _handle, nint.Zero);
        if (handle != nint.Zero) NativeMethods.DestroyLayer(handle);
    }
}
