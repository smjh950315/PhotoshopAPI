using System.Runtime.InteropServices;
using System.Text;

namespace Photoshop.NET;

/// <summary>
/// Provides a temporary null-terminated UTF-8 buffer for native calls.
/// </summary>
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
