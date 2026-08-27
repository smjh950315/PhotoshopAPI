namespace Photoshop.NET;

/// <summary>Editable common properties shared by all layer types.</summary>
public readonly record struct PhotoshopLayerInfo(
    PhotoshopLayerType Type,
    PhotoshopBitDepth BitDepth,
    PhotoshopColorMode ColorMode,
    PhotoshopBlendMode BlendMode,
    PhotoshopLayerColor DisplayColor,
    uint Width,
    uint Height,
    float CenterX,
    float CenterY,
    float Opacity,
    float Fill,
    bool Visible,
    bool Locked,
    bool ClippingMask);
