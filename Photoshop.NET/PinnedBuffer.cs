using System.Runtime.InteropServices;

namespace Photoshop.NET;

/// <summary>
/// Pins a managed byte array while it is passed to the native API.
/// </summary>
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
