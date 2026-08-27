namespace Photoshop.NET;

/// <summary>Layer blend mode. Values map directly to the native PhotoshopAPI enum.</summary>
public enum PhotoshopBlendMode
{
    Passthrough, Normal, Dissolve, Darken, Multiply, ColorBurn, LinearBurn, DarkerColor,
    Lighten, Screen, ColorDodge, LinearDodge, LighterColor, Overlay, SoftLight, HardLight,
    VividLight, LinearLight, PinLight, HardMix, Difference, Exclusion, Subtract, Divide,
    Hue, Saturation, Color, Luminosity
}
