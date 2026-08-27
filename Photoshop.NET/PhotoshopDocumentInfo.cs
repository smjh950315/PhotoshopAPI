namespace Photoshop.NET;

/// <summary>Immutable snapshot of document metadata.</summary>
public readonly record struct PhotoshopDocumentInfo(
    ulong Width,
    ulong Height,
    float Dpi,
    PhotoshopBitDepth BitDepth,
    PhotoshopColorMode ColorMode,
    uint RootLayerCount);
