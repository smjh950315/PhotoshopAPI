namespace Photoshop.NET;

/// <summary>
/// Owns the temporary native representation of managed layer options.
/// </summary>
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
