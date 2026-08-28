namespace Photoshop.NET;

/// <summary>Owns a native Photoshop document and its layer hierarchy.</summary>
public sealed class PhotoshopDocument : IDisposable
{
    private nint _handle;

    private PhotoshopDocument(nint handle) => _handle = handle;

    /// <summary>Gets whether this wrapper has released its native handle.</summary>
    public bool IsDisposed => _handle == nint.Zero;
    /// <summary>Gets the most recent detailed native error.</summary>
    public string LastError => IsDisposed ? string.Empty : NativeMethods.GetDocumentError(_handle);
    /// <summary>Gets a current metadata snapshot.</summary>
    public PhotoshopDocumentInfo Info
    {
        get
        {
            EnsureUsable();
            var status = NativeMethods.GetDocumentInfo(_handle, out var info);
            Throw(status);
            return new(info.Width, info.Height, info.Dpi, info.BitDepth, info.ColorMode, info.RootLayerCount);
        }
    }

    /// <summary>Creates an 8-bit RGB document.</summary>
    public static PhotoshopDocument Create(uint width, uint height) =>
        Create(PhotoshopBitDepth.Bit8, PhotoshopColorMode.Rgb, width, height);

    /// <summary>Creates a document with an explicit sample depth and color mode.</summary>
    public static PhotoshopDocument Create(PhotoshopBitDepth depth, PhotoshopColorMode mode, ulong width, ulong height)
    {
        PhotoshopApi.EnsureCompatible();
        var status = NativeMethods.CreateDocumentEx(depth, mode, width, height, out var handle);
        PhotoshopApi.ThrowIfFailed(status);
        return new(handle);
    }

    /// <summary>Reads a PSD or PSB and automatically detects 8-, 16-, or 32-bit samples.</summary>
    public static PhotoshopDocument Read(string path)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(path);
        PhotoshopApi.EnsureCompatible();
        using var utf8 = new Utf8Buffer(path);
        var status = NativeMethods.ReadDocument(utf8.Pointer, out var handle);
        PhotoshopApi.ThrowIfFailed(status);
        return new(handle);
    }

    /// <summary>Changes the canvas dimensions.</summary>
    public void SetSize(ulong width, ulong height) { EnsureUsable(); Throw(NativeMethods.SetDocumentSize(_handle, width, height)); }
    /// <summary>Changes the document resolution in dots per inch.</summary>
    public void SetDpi(float dpi) { EnsureUsable(); Throw(NativeMethods.SetDocumentDpi(_handle, dpi)); }
    /// <summary>Selects compression for all channels on the next write.</summary>
    public void SetCompression(PhotoshopCompression compression) { EnsureUsable(); Throw(NativeMethods.SetDocumentCompression(_handle, compression)); }
    /// <summary>Sets the merged RGB8 composite written in the document image-data section.</summary>
    public void SetMergedImage(Rgb8Image image)
    {
        EnsureUsable();
        PhotoshopApi.ValidateImageBuffer(image.Pixels.Span, image.Width, image.Height, image.StrideBytes, 3, nameof(image));
        var info = Info;
        if (info.BitDepth != PhotoshopBitDepth.Bit8 || info.ColorMode != PhotoshopColorMode.Rgb)
        {
            throw new NotSupportedException("Merged RGB8 images require an 8-bit RGB document.");
        }
        if (image.Width != info.Width || image.Height != info.Height)
        {
            throw new ArgumentException("Merged image dimensions must match the document dimensions.", nameof(image));
        }
        var copy = image.Pixels.ToArray();
        using var pinned = new PinnedBuffer(copy);
        var view = new NativeRgb8View(pinned.Pointer, image.Width, image.Height, image.StrideBytes);
        Throw(NativeMethods.SetDocumentMergedRgb8(_handle, ref view));
    }
    /// <summary>Invalidates cached text previews so Photoshop renders edited text.</summary>
    public void InvalidateTextCache() { EnsureUsable(); Throw(NativeMethods.InvalidateTextCache(_handle)); }

    /// <summary>Reads the embedded ICC profile bytes.</summary>
    public byte[] GetIccProfile()
    {
        EnsureUsable();
        return NativeMethods.ReadBytes((nint b, ulong c, out ulong r) => NativeMethods.GetIccProfile(_handle, b, c, out r));
    }

    /// <summary>Replaces the embedded ICC profile; an empty span clears it.</summary>
    public void SetIccProfile(ReadOnlySpan<byte> profile)
    {
        EnsureUsable();
        var copy = profile.ToArray();
        if (copy.Length == 0)
        {
            Throw(NativeMethods.SetIccProfile(_handle, nint.Zero, 0));
            return;
        }
        using var pinned = new PinnedBuffer(copy);
        Throw(NativeMethods.SetIccProfile(_handle, pinned.Pointer, (ulong)copy.Length));
    }

    /// <summary>Returns one root layer by zero-based index.</summary>
    public PhotoshopLayer GetRootLayer(uint index)
    {
        EnsureUsable();
        var status = NativeMethods.GetRootLayer(_handle, index, out var layer);
        Throw(status);
        return PhotoshopLayer.FromHandle(layer);
    }

    /// <summary>Returns all current root layers.</summary>
    public IReadOnlyList<PhotoshopLayer> GetRootLayers()
    {
        var count = Info.RootLayerCount;
        var result = new List<PhotoshopLayer>(checked((int)count));
        for (uint index = 0; index < count; ++index) result.Add(GetRootLayer(index));
        return result;
    }

    /// <summary>Finds a layer using a slash-separated path such as <c>Group/Image</c>.</summary>
    public PhotoshopLayer FindLayer(string path)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(path);
        EnsureUsable();
        using var utf8 = new Utf8Buffer(path);
        var status = NativeMethods.FindLayer(_handle, utf8.Pointer, out var layer);
        Throw(status);
        return PhotoshopLayer.FromHandle(layer);
    }

    /// <summary>Adds a detached layer at document root.</summary>
    public void AddLayer(PhotoshopLayer layer)
    {
        ArgumentNullException.ThrowIfNull(layer);
        EnsureUsable();
        layer.EnsureUsable();
        Throw(NativeMethods.DocumentAddLayer(_handle, layer.Handle));
    }

    /// <summary>Removes a layer from the hierarchy without invalidating its wrapper.</summary>
    public void RemoveLayer(PhotoshopLayer layer)
    {
        ArgumentNullException.ThrowIfNull(layer);
        EnsureUsable();
        layer.EnsureUsable();
        Throw(NativeMethods.DocumentRemoveLayer(_handle, layer.Handle));
    }

    /// <summary>Moves a layer under a group, or to root when <paramref name="newParent"/> is null.</summary>
    public void MoveLayer(PhotoshopLayer layer, PhotoshopGroupLayer? newParent = null)
    {
        ArgumentNullException.ThrowIfNull(layer);
        EnsureUsable();
        layer.EnsureUsable();
        newParent?.EnsureUsable();
        Throw(NativeMethods.DocumentMoveLayer(_handle, layer.Handle, newParent?.Handle ?? nint.Zero));
    }

    /// <summary>Writes the document and consumes its native document state.</summary>
    public void Write(string path, bool overwrite = true)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(path);
        EnsureUsable();
        using var utf8 = new Utf8Buffer(path);
        var status = NativeMethods.DocumentWriteEx(_handle, utf8.Pointer, overwrite ? (byte)1 : (byte)0);
        Throw(status);
        ReleaseHandle();
    }

    /// <inheritdoc />
    public void Dispose() { ReleaseHandle(); GC.SuppressFinalize(this); }
    ~PhotoshopDocument() => ReleaseHandle();

    private void Throw(PhotoshopStatus status) => PhotoshopApi.ThrowIfFailed(status, LastError);
    private void EnsureUsable() => ObjectDisposedException.ThrowIf(IsDisposed, this);
    private void ReleaseHandle()
    {
        var handle = Interlocked.Exchange(ref _handle, nint.Zero);
        if (handle != nint.Zero) NativeMethods.DestroyDocument(handle);
    }
}
