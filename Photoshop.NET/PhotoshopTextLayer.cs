namespace Photoshop.NET;

/// <summary>Text layer loaded from an existing PSD or PSB.</summary>
public sealed class PhotoshopTextLayer : PhotoshopLayer
{
    internal PhotoshopTextLayer(nint handle) : base(handle) { }

    /// <summary>Gets or replaces the UTF-8 text payload.</summary>
    public string Text
    {
        get
        {
            EnsureUsable();
            return NativeMethods.ReadString((nint b, uint c, out uint r) => NativeMethods.GetText(Handle, b, c, out r));
        }
        set
        {
            ArgumentNullException.ThrowIfNull(value);
            EnsureUsable();
            using var utf8 = new Utf8Buffer(value);
            Throw(NativeMethods.SetText(Handle, utf8.Pointer));
        }
    }

    /// <summary>Replaces matching text while preserving the layer's text styling structures.</summary>
    public void ReplaceText(string oldText, string newText, bool replaceAll = true)
    {
        ArgumentException.ThrowIfNullOrEmpty(oldText);
        ArgumentNullException.ThrowIfNull(newText);
        EnsureUsable();
        using var oldUtf8 = new Utf8Buffer(oldText);
        using var newUtf8 = new Utf8Buffer(newText);
        Throw(NativeMethods.ReplaceText(Handle, oldUtf8.Pointer, newUtf8.Pointer, replaceAll ? (byte)1 : (byte)0));
    }
}
