# Photoshop.NET

`Photoshop.NET` is a dependency-free .NET 8 wrapper over the version 2
`PhotoshopAPI.C` ABI. Native objects use deterministic `IDisposable` ownership;
dispose every document and layer handle returned by the wrapper.

```csharp
using var document = PhotoshopDocument.Read("input.psd");

foreach (var layer in document.GetRootLayers())
{
    using (layer)
    {
        Console.WriteLine($"{layer.Info.Type}: {layer.Name}");
    }
}

using var title = document.FindLayer("Artwork/Title");
if (title is PhotoshopTextLayer text)
{
    text.ReplaceText("Draft", "Final");
    document.InvalidateTextCache();
}

document.Write("output.psd");
```

The wrapper automatically detects 8-, 16-, and 32-bit files when reading.
Channel and mask access is explicit about sample representation:

- `GetChannel8` / `SetChannel8` use `byte` samples.
- `GetChannel16` / `SetChannel16` use `ushort` samples.
- `GetChannel32` / `SetChannel32` use `float` samples.
- `GetChannelBytes` and `GetMaskBytes` expose the exact native bytes when a
  serializer or zero-copy bridge needs to choose the interpretation itself.

Runtime layer handles are returned as `PhotoshopGroupLayer`,
`PhotoshopImageLayer`, `PhotoshopTextLayer`, or `PhotoshopSmartObjectLayer`
when the native type is known. Shape, adjustment, artboard, and section layers
retain their common state through `PhotoshopLayer`; their original tagged blocks are
preserved by the C++ library when the document is written.
