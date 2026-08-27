namespace Photoshop.NET;

/// <summary>
/// Describes the initial properties of a Photoshop layer.
/// </summary>
/// <param name="Name">The optional layer name.</param>
/// <param name="Left">The layer's left position.</param>
/// <param name="Top">The layer's top position.</param>
/// <param name="Opacity">The layer opacity from 0 to 1.</param>
/// <param name="Visible">Whether the layer is initially visible.</param>
/// <param name="Locked">Whether the layer is initially locked.</param>
public readonly record struct PhotoshopLayerOptions(
    string? Name = null,
    int Left = 0,
    int Top = 0,
    float Opacity = 1.0f,
    bool Visible = true,
    bool Locked = false);
