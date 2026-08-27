namespace Photoshop.NET;

/// <summary>
/// Represents a Photoshop document owned by the native library.
/// </summary>
public sealed class PhotoshopDocument : IDisposable
{
    private nint _handle;

    private PhotoshopDocument(nint handle)
    {
        _handle = handle;
    }

    /// <summary>
    /// Gets a value indicating whether the document has been disposed.
    /// </summary>
    public bool IsDisposed => _handle == nint.Zero;

    /// <summary>
    /// Gets the most recent native error for this document.
    /// </summary>
    public string LastError =>
        _handle == nint.Zero ? string.Empty : NativeMethods.GetDocumentError(_handle);

    /// <summary>
    /// Creates a new document with the specified dimensions.
    /// </summary>
    /// <param name="width">The document width in pixels.</param>
    /// <param name="height">The document height in pixels.</param>
    /// <returns>A new document handle.</returns>
    /// <exception cref="PhotoshopApiException">Thrown when native creation fails.</exception>
    public static PhotoshopDocument Create(uint width, uint height)
    {
        PhotoshopApi.EnsureCompatible();
        var status = NativeMethods.CreateDocument(width, height, out var handle);
        PhotoshopApi.ThrowIfFailed(status);
        return new PhotoshopDocument(handle);
    }

    /// <summary>
    /// Adds a layer to this document.
    /// </summary>
    /// <param name="layer">The layer to add.</param>
    public void AddLayer(PhotoshopLayer layer)
    {
        ArgumentNullException.ThrowIfNull(layer);
        EnsureUsable();
        layer.EnsureUsable();

        var status = NativeMethods.DocumentAddLayer(_handle, layer.Handle);
        PhotoshopApi.ThrowIfFailed(status, LastError);
    }

    /// <summary>
    /// Writes the document to a PSD file and releases its native handle.
    /// </summary>
    /// <param name="path">The destination PSD path.</param>
    public void Write(string path)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(path);
        EnsureUsable();

        using var utf8Path = new Utf8Buffer(path);
        var status = NativeMethods.DocumentWrite(_handle, utf8Path.Pointer);
        PhotoshopApi.ThrowIfFailed(status, LastError);
        ReleaseHandle();
    }

    /// <summary>
    /// Releases the native document handle.
    /// </summary>
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
