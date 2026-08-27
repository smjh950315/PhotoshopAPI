namespace Photoshop.NET;

/// <summary>Photoshop group or artboard layer with child traversal.</summary>
public sealed class PhotoshopGroupLayer : PhotoshopLayer
{
    internal PhotoshopGroupLayer(nint handle) : base(handle) { }

    /// <summary>Gets or sets whether Photoshop displays the group collapsed.</summary>
    public bool Collapsed
    {
        get { EnsureUsable(); var status = NativeMethods.GetCollapsed(Handle, out var value); Throw(status); return value != 0; }
        set { EnsureUsable(); Throw(NativeMethods.SetCollapsed(Handle, value ? (byte)1 : (byte)0)); }
    }

    /// <summary>Gets the number of direct child layers.</summary>
    public uint ChildCount
    {
        get { EnsureUsable(); var status = NativeMethods.GetChildCount(Handle, out var count); Throw(status); return count; }
    }

    /// <summary>Returns one direct child by zero-based index.</summary>
    public PhotoshopLayer GetChild(uint index)
    {
        EnsureUsable();
        var status = NativeMethods.GetChild(Handle, index, out var child);
        Throw(status);
        return FromHandle(child);
    }

    /// <summary>Returns all current direct children.</summary>
    public IReadOnlyList<PhotoshopLayer> GetChildren()
    {
        var count = ChildCount;
        var result = new List<PhotoshopLayer>(checked((int)count));
        for (uint index = 0; index < count; ++index) result.Add(GetChild(index));
        return result;
    }

    /// <summary>Adds a detached child to an attached 8-bit group.</summary>
    public void AddChild(PhotoshopLayer child)
    {
        ArgumentNullException.ThrowIfNull(child);
        EnsureUsable();
        child.EnsureUsable();
        Throw(NativeMethods.GroupAddLayer(Handle, child.Handle));
    }

    internal static PhotoshopGroupLayer Create(PhotoshopLayerOptions options)
    {
        using var nativeOptions = NativeLayerOptions.Create(options);
        var value = nativeOptions.Value;
        var status = NativeMethods.CreateGroupLayer(ref value, out var handle);
        PhotoshopApi.ThrowIfFailed(status);
        return new(handle);
    }
}
