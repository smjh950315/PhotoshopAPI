namespace Photoshop.NET;

/// <summary>Runtime kind of a Photoshop layer.</summary>
public enum PhotoshopLayerType
{
    Unknown = 0,
    Image = 1,
    Group = 2,
    Text = 3,
    SmartObject = 4,
    Shape = 5,
    Adjustment = 6,
    Artboard = 7,
    SectionDivider = 8
}
