namespace Photoshop.NET;

/// <summary>Smart-object layer with linked-file identity and non-destructive transforms.</summary>
public sealed class PhotoshopSmartObjectLayer : PhotoshopLayer
{
    internal PhotoshopSmartObjectLayer(nint handle) : base(handle) { }

    /// <summary>Gets the linked-data hash used by the Photoshop document.</summary>
    public string Hash => Read((nint b, uint c, out uint r) => NativeMethods.GetSmartHash(Handle, b, c, out r));
    /// <summary>Gets the linked image filename.</summary>
    public string Filename => Read((nint b, uint c, out uint r) => NativeMethods.GetSmartFilename(Handle, b, c, out r));
    /// <summary>Gets the linked image path as UTF-8 text.</summary>
    public string Filepath => Read((nint b, uint c, out uint r) => NativeMethods.GetSmartFilepath(Handle, b, c, out r));

    /// <summary>Replaces linked image data while preserving transforms and warp.</summary>
    public void Replace(string path, bool linkExternally = false)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(path);
        EnsureUsable();
        using var utf8 = new Utf8Buffer(path);
        Throw(NativeMethods.ReplaceSmartObject(Handle, utf8.Pointer, linkExternally ? (byte)1 : (byte)0));
    }

    /// <summary>Moves the smart object in document coordinates.</summary>
    public void Move(double xOffset, double yOffset) { EnsureUsable(); Throw(NativeMethods.MoveSmartObject(Handle, xOffset, yOffset)); }
    /// <summary>Rotates around the layer center in degrees.</summary>
    public void Rotate(double angleDegrees) { EnsureUsable(); Throw(NativeMethods.RotateSmartObject(Handle, angleDegrees)); }
    /// <summary>Scales independently around the layer center.</summary>
    public void Scale(double xFactor, double yFactor) { EnsureUsable(); Throw(NativeMethods.ScaleSmartObject(Handle, xFactor, yFactor)); }
    /// <summary>Resets affine transformations while retaining warp.</summary>
    public void ResetTransform() { EnsureUsable(); Throw(NativeMethods.ResetSmartTransform(Handle)); }
    /// <summary>Resets warp while retaining affine transformations.</summary>
    public void ResetWarp() { EnsureUsable(); Throw(NativeMethods.ResetSmartWarp(Handle)); }

    private string Read(NativeMethods.StringReader reader)
    {
        EnsureUsable();
        return NativeMethods.ReadString(reader);
    }
}
