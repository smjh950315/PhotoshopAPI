namespace Photoshop.NET;

/// <summary>Layer-mask geometry and Photoshop mask flags.</summary>
public readonly record struct PhotoshopMaskInfo(
    ulong Width,
    ulong Height,
    double CenterX,
    double CenterY,
    double? Feather,
    bool HasMask,
    bool Disabled,
    bool RelativeToLayer,
    byte DefaultColor,
    byte? Density);
