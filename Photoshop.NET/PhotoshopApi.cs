using System.Runtime.InteropServices;
using System.Text;

namespace Photoshop.NET;

public enum PhotoshopStatus
{
    Success = 0,
    InvalidArgument = 1,
    DimensionsInvalid = 2,
    DimensionsMismatch = 3,
    AllocationFailed = 4,
    OwnershipError = 5,
    IoError = 6,
    WriteFailed = 7,
    NotSupported = 8,
    BufferTooSmall = 9
}

public sealed class PhotoshopApiException : Exception
{
    public PhotoshopApiException(PhotoshopStatus status, string message)
        : base(message)
    {
        Status = status;
    }

    public PhotoshopStatus Status { get; }
}

public readonly record struct PhotoshopLayerOptions(
    string? Name = null,
    int Left = 0,
    int Top = 0,
    float Opacity = 1.0f,
    bool Visible = true,
    bool Locked = false);

public readonly record struct Rgba8Image(
    ReadOnlyMemory<byte> Pixels,
    uint Width,
    uint Height,
    uint StrideBytes = 0);

public readonly record struct Mask8Image(
    ReadOnlyMemory<byte> Pixels,
    uint Width,
    uint Height,
    uint StrideBytes = 0);

public static class PhotoshopApi
{
    public const uint AbiVersion = 1;

    public static uint NativeAbiVersion => NativeMethods.GetAbiVersion();

    public static void EnsureCompatible()
    {
        var version = NativeAbiVersion;
        if (version != AbiVersion)
        {
            throw new PhotoshopApiException(
                PhotoshopStatus.NotSupported,
                $"PhotoshopAPI.C ABI version {version} is not supported; expected {AbiVersion}.");
        }
    }

    internal static void ThrowIfFailed(PhotoshopStatus status, string? nativeError = null)
    {
        if (status != PhotoshopStatus.Success)
        {
            throw new PhotoshopApiException(status, string.IsNullOrWhiteSpace(nativeError)
                ? GetDefaultMessage(status)
                : nativeError);
        }
    }

    internal static void ValidateImageBuffer(
        ReadOnlySpan<byte> pixels,
        uint width,
        uint height,
        uint strideBytes,
        uint bytesPerPixel,
        string parameterName)
    {
        if (width == 0 || height == 0)
        {
            throw new ArgumentOutOfRangeException(parameterName, "Width and height must be non-zero.");
        }

        var minimumStride = checked((ulong)width * bytesPerPixel);
        var stride = strideBytes == 0 ? minimumStride : strideBytes;
        if (stride < minimumStride)
        {
            throw new ArgumentException("Stride is smaller than one packed row.", parameterName);
        }

        var requiredBytes = checked((height - 1UL) * stride + minimumStride);
        if (requiredBytes > (ulong)pixels.Length)
        {
            throw new ArgumentException(
                $"The buffer contains {pixels.Length} bytes, but at least {requiredBytes} are required.",
                parameterName);
        }
    }

    private static string GetDefaultMessage(PhotoshopStatus status) => status switch
    {
        PhotoshopStatus.InvalidArgument => "The native API rejected an argument.",
        PhotoshopStatus.DimensionsInvalid => "The supplied dimensions are invalid.",
        PhotoshopStatus.DimensionsMismatch => "The supplied dimensions do not match.",
        PhotoshopStatus.AllocationFailed => "The native API could not allocate memory.",
        PhotoshopStatus.OwnershipError => "The native object is no longer usable or belongs to another document.",
        PhotoshopStatus.IoError => "The native API could not access the requested file.",
        PhotoshopStatus.WriteFailed => "The native API failed to write the document.",
        PhotoshopStatus.NotSupported => "The requested operation is not supported.",
        PhotoshopStatus.BufferTooSmall => "The supplied buffer is too small.",
        _ => $"PhotoshopAPI.C returned status {(int)status}."
    };
}

public sealed class PhotoshopDocument : IDisposable
{
    private nint _handle;

    private PhotoshopDocument(nint handle)
    {
        _handle = handle;
    }

    public bool IsDisposed => _handle == nint.Zero;

    public string LastError =>
        _handle == nint.Zero ? string.Empty : NativeMethods.GetDocumentError(_handle);

    public static PhotoshopDocument Create(uint width, uint height)
    {
        PhotoshopApi.EnsureCompatible();
        var status = NativeMethods.CreateDocument(width, height, out var handle);
        PhotoshopApi.ThrowIfFailed(status);
        return new PhotoshopDocument(handle);
    }

    public void AddLayer(PhotoshopLayer layer)
    {
        ArgumentNullException.ThrowIfNull(layer);
        EnsureUsable();
        layer.EnsureUsable();

        var status = NativeMethods.DocumentAddLayer(_handle, layer.Handle);
        PhotoshopApi.ThrowIfFailed(status, LastError);
    }

    public void Write(string path)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(path);
        EnsureUsable();

        using var utf8Path = new Utf8Buffer(path);
        var status = NativeMethods.DocumentWrite(_handle, utf8Path.Pointer);
        PhotoshopApi.ThrowIfFailed(status, LastError);
        ReleaseHandle();
    }

    public void Dispose()
    {
        ReleaseHandle();
        GC.SuppressFinalize(this);
    }

    ~PhotoshopDocument()
    {
        ReleaseHandle();
    }

    private void EnsureUsable()
    {
        ObjectDisposedException.ThrowIf(IsDisposed, this);
    }

    private void ReleaseHandle()
    {
        var handle = Interlocked.Exchange(ref _handle, nint.Zero);
        if (handle != nint.Zero)
        {
            NativeMethods.DestroyDocument(handle);
        }
    }
}

public sealed class PhotoshopLayer : IDisposable
{
    private nint _handle;

    private PhotoshopLayer(nint handle)
    {
        _handle = handle;
    }

    public bool IsDisposed => _handle == nint.Zero;

    public string LastError =>
        _handle == nint.Zero ? string.Empty : NativeMethods.GetLayerError(_handle);

    internal nint Handle => _handle;

    public static PhotoshopLayer CreateGroup(PhotoshopLayerOptions options = default)
    {
        using var nativeOptions = NativeLayerOptions.Create(options);
        var optionsValue = nativeOptions.Value;
        var status = NativeMethods.CreateGroupLayer(ref optionsValue, out var handle);
        PhotoshopApi.ThrowIfFailed(status);
        return new PhotoshopLayer(handle);
    }

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

    public void SetMask8(Mask8Image image)
    {
        SetMask8(image.Pixels.Span, image.Width, image.Height, image.StrideBytes);
    }

    public void AddChild(PhotoshopLayer child)
    {
        ArgumentNullException.ThrowIfNull(child);
        EnsureUsable();
        child.EnsureUsable();

        var status = NativeMethods.GroupAddLayer(_handle, child.Handle);
        PhotoshopApi.ThrowIfFailed(status, LastError);
    }

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

internal sealed class NativeLayerOptions : IDisposable
{
    private readonly Utf8Buffer _name;

    private NativeLayerOptions(Utf8Buffer name, PhotoshopLayerOptions options)
    {
        _name = name;
        Value = new NativeLayerOptionsData
        {
            Name = name.Pointer,
            Left = options.Left,
            Top = options.Top,
            Opacity = options.Opacity,
            Visible = options.Visible ? (byte)1 : (byte)0,
            Locked = options.Locked ? (byte)1 : (byte)0,
            Reserved0 = 0,
            Reserved1 = 0
        };
    }

    public NativeLayerOptionsData Value { get; }

    public static NativeLayerOptions Create(PhotoshopLayerOptions options)
    {
        return new NativeLayerOptions(new Utf8Buffer(options.Name), options);
    }

    public void Dispose()
    {
        _name.Dispose();
    }
}

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

[StructLayout(LayoutKind.Sequential)]
internal struct NativeRgba8View
{
    public NativeRgba8View(nint pixels, uint width, uint height, uint strideBytes)
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

internal sealed class Utf8Buffer : IDisposable
{
    private GCHandle _handle;

    public Utf8Buffer(string? value)
    {
        if (value is null)
        {
            return;
        }

        var encoded = Encoding.UTF8.GetBytes(value);
        var bytes = new byte[encoded.Length + 1];
        encoded.CopyTo(bytes, 0);
        _handle = GCHandle.Alloc(bytes, GCHandleType.Pinned);
        Pointer = _handle.AddrOfPinnedObject();
    }

    public nint Pointer { get; }

    public void Dispose()
    {
        if (_handle.IsAllocated)
        {
            _handle.Free();
        }
        GC.SuppressFinalize(this);
    }
}

internal sealed class PinnedBuffer : IDisposable
{
    private readonly GCHandle _handle;

    public PinnedBuffer(byte[] bytes)
    {
        if (bytes.Length == 0)
        {
            throw new ArgumentException("The buffer must not be empty.", nameof(bytes));
        }

        _handle = GCHandle.Alloc(bytes, GCHandleType.Pinned);
        Pointer = _handle.AddrOfPinnedObject();
    }

    public nint Pointer { get; }

    public void Dispose()
    {
        _handle.Free();
        GC.SuppressFinalize(this);
    }
}

internal static class NativeMethods
{
    private const string LibraryName = "PhotoshopAPI.C";

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true, EntryPoint = "photoshopapi_c_get_abi_version")]
    internal static extern uint GetAbiVersion();

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true, EntryPoint = "photoshopapi_c_document_create")]
    internal static extern PhotoshopStatus CreateDocument(uint width, uint height, out nint document);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true, EntryPoint = "photoshopapi_c_document_destroy")]
    internal static extern void DestroyDocument(nint document);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true, EntryPoint = "photoshopapi_c_group_layer_create")]
    internal static extern PhotoshopStatus CreateGroupLayer(
        ref NativeLayerOptionsData options,
        out nint layer);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true, EntryPoint = "photoshopapi_c_image_layer_create_rgba8")]
    internal static extern PhotoshopStatus CreateImageLayer(
        ref NativeRgba8View source,
        ref NativeLayerOptionsData options,
        out nint layer);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true, EntryPoint = "photoshopapi_c_layer_destroy")]
    internal static extern void DestroyLayer(nint layer);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true, EntryPoint = "photoshopapi_c_document_add_layer")]
    internal static extern PhotoshopStatus DocumentAddLayer(nint document, nint layer);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true, EntryPoint = "photoshopapi_c_group_add_layer")]
    internal static extern PhotoshopStatus GroupAddLayer(nint group, nint child);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true, EntryPoint = "photoshopapi_c_layer_set_mask8")]
    internal static extern PhotoshopStatus SetMask(nint layer, ref NativeMask8View mask);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true, EntryPoint = "photoshopapi_c_document_write")]
    internal static extern PhotoshopStatus DocumentWrite(nint document, nint utf8Path);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true, EntryPoint = "photoshopapi_c_document_get_last_error")]
    private static extern PhotoshopStatus GetDocumentErrorNative(
        nint document,
        nint buffer,
        uint bufferCapacity,
        out uint requiredBufferSize);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true, EntryPoint = "photoshopapi_c_layer_get_last_error")]
    private static extern PhotoshopStatus GetLayerErrorNative(
        nint layer,
        nint buffer,
        uint bufferCapacity,
        out uint requiredBufferSize);

    internal static string GetDocumentError(nint document) => ReadError(
        (nint buffer, uint capacity, out uint required) =>
            GetDocumentErrorNative(document, buffer, capacity, out required));

    internal static string GetLayerError(nint layer) => ReadError(
        (nint buffer, uint capacity, out uint required) =>
            GetLayerErrorNative(layer, buffer, capacity, out required));

    private static string ReadError(ErrorReader reader)
    {
        var status = reader(nint.Zero, 0, out var required);
        if (status != PhotoshopStatus.Success || required == 0 || required > int.MaxValue)
        {
            return string.Empty;
        }

        var buffer = new byte[(int)required];
        var pinned = GCHandle.Alloc(buffer, GCHandleType.Pinned);
        try
        {
            status = reader(pinned.AddrOfPinnedObject(), required, out var actualRequired);
            if (status != PhotoshopStatus.Success || actualRequired > required)
            {
                return string.Empty;
            }
            return Encoding.UTF8.GetString(buffer, 0, Math.Max(0, (int)actualRequired - 1));
        }
        finally
        {
            pinned.Free();
        }
    }

    private delegate PhotoshopStatus ErrorReader(nint buffer, uint capacity, out uint required);
}
