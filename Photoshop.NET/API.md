# Photoshop.NET API reference and parity audit

This document describes the public .NET 8 API implemented by `Photoshop.NET`
and compares it with the high-level C++ API documented by the upstream
[PhotoshopAPI documentation](https://photoshopapi.readthedocs.io/en/latest/index.html).
The comparison was reviewed on 2026-08-27 against the repository source and
the upstream pages for
[`LayeredFile`](https://photoshopapi.readthedocs.io/en/latest/code/layeredfile.html),
[`Layer`](https://photoshopapi.readthedocs.io/en/latest/code/layers/baselayer.html),
[`GroupLayer`](https://photoshopapi.readthedocs.io/en/latest/code/layers/group.html),
[`ImageLayer`](https://photoshopapi.readthedocs.io/en/latest/code/layers/image.html),
[`SmartObjectLayer`](https://photoshopapi.readthedocs.io/en/latest/code/layers/smartobject.html),
and [editable text layers](https://photoshopapi.readthedocs.io/en/latest/concepts/text-layers.html).

`Photoshop.NET` wraps the stable, opaque-handle C ABI in `PhotoshopAPI.C`; it
does not directly expose C++ templates, standard-library objects, compressed
channel objects, tagged blocks, or raw `PhotoshopFile` parser structures.

## Lifetime and error rules

- Every `PhotoshopDocument` and `PhotoshopLayer` owns a native handle and must
  be disposed. Prefer `using` declarations or `using` statements.
- Layer wrappers own shared references to native layers. Disposing a wrapper
  does not remove the layer from its document.
- `PhotoshopDocument.Write()` consumes the native document state, matching the
  C++ `LayeredFile::write()` contract. The managed document is disposed after a
  successful write.
- Native failures throw `PhotoshopApiException`. Its `Status` identifies the
  error category; `LastError` on documents and layers provides native detail
  while the handle remains valid.
- A document or layer handle is not internally synchronized. Serialize access
  to one handle when using multiple threads.

## Quick start

```csharp
using Photoshop.NET;

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

## `PhotoshopApi`

```csharp
public static class PhotoshopApi
{
    public const uint AbiVersion = 3;
    public static uint NativeAbiVersion { get; }
    public static void EnsureCompatible();
}
```

Call `EnsureCompatible()` before direct native deployment diagnostics. Document
creation and reading call it automatically.

## `PhotoshopDocument`

### Construction and metadata

```csharp
public static PhotoshopDocument Create(uint width, uint height);
public static PhotoshopDocument Create(
    PhotoshopBitDepth depth,
    PhotoshopColorMode mode,
    ulong width,
    ulong height);
public static PhotoshopDocument Read(string path);

public PhotoshopDocumentInfo Info { get; }
public bool IsDisposed { get; }
public string LastError { get; }
```

`Read()` accepts PSD and PSB files and automatically selects 8-, 16-, or
32-bit native storage. The four-argument `Create()` can create document
metadata at every supported depth, but the current layer constructors only
create 8-bit layers. Therefore creating a useful 16- or 32-bit document from
scratch is a known parity gap; reading and editing existing 16-/32-bit layers
is supported.

`PhotoshopDocumentInfo` contains `Width`, `Height`, `Dpi`, `BitDepth`,
`ColorMode`, and `RootLayerCount`.

### Document editing

```csharp
public void SetSize(ulong width, ulong height);
public void SetDpi(float dpi);
public void SetCompression(PhotoshopCompression compression);
public void SetMergedImage(Rgb8Image image);
public void InvalidateTextCache();

public byte[] GetIccProfile();
public void SetIccProfile(ReadOnlySpan<byte> profile);
```

`SetCompression()` selects one write compression for all current channels.
`SetMergedImage()` supplies the flattened RGB8 composite stored in the PSD
image-data section. The image dimensions must match an 8-bit RGB document.
After editing text from a file read from disk, call `InvalidateTextCache()`
before writing so Photoshop refreshes the rendered text.

### Hierarchy

```csharp
public PhotoshopLayer GetRootLayer(uint index);
public IReadOnlyList<PhotoshopLayer> GetRootLayers();
public PhotoshopLayer FindLayer(string path);

public void AddLayer(PhotoshopLayer layer);
public void RemoveLayer(PhotoshopLayer layer);
public void MoveLayer(PhotoshopLayer layer, PhotoshopGroupLayer? newParent = null);
```

Paths use `/` separators, for example `"Group/Nested/Image"`. Returned handles
are runtime-typed as `PhotoshopGroupLayer`, `PhotoshopImageLayer`,
`PhotoshopTextLayer`, or `PhotoshopSmartObjectLayer` when applicable.

### Output

```csharp
public void Write(string path, bool overwrite = true);
```

The call consumes and disposes the document after a successful write.

## `PhotoshopLayer`

### Common properties

```csharp
public PhotoshopLayerInfo Info { get; set; }
public string Name { get; set; }
public bool IsDisposed { get; }
public string LastError { get; }
```

`PhotoshopLayerInfo` reports:

- runtime `Type`, `BitDepth`, and `ColorMode`;
- `BlendMode` and `DisplayColor`;
- `Width`, `Height`, `CenterX`, and `CenterY`;
- `Opacity`, `Fill`, `Visible`, `Locked`, and `ClippingMask`.

When assigning `Info`, `Type`, `BitDepth`, and `ColorMode` are identity fields
and are ignored by the native setter. All other fields are applied. Opacity and
fill use the range 0.0 through 1.0.

### Channel access

```csharp
public IReadOnlyList<int> GetChannelIndices();
public byte[] GetChannelBytes(int channelIndex);
public byte[] GetChannel8(int channelIndex);
public ushort[] GetChannel16(int channelIndex);
public float[] GetChannel32(int channelIndex);

public void SetChannelBytes(int channelIndex, ReadOnlySpan<byte> samples);
public void SetChannel8(int channelIndex, ReadOnlySpan<byte> samples);
public void SetChannel16(int channelIndex, ReadOnlySpan<ushort> samples);
public void SetChannel32(int channelIndex, ReadOnlySpan<float> samples);
```

Photoshop channel indices are passed as integers: RGB channels are 0, 1, and
2; alpha is -1; a pixel mask is -2. Smart-object and rendered-text image data
is read-only in the upstream API, so channel setters on those layer types fail
with `PhotoshopStatus.TypeMismatch`.

### Pixel masks

```csharp
public PhotoshopMaskInfo GetMaskInfo();
public void SetMaskInfo(PhotoshopMaskInfo value);
public byte[] GetMaskBytes();
public void SetMaskBytes(ReadOnlySpan<byte> samples, ulong width, ulong height);
public void SetMask8(ReadOnlySpan<byte> pixels, uint width, uint height, uint strideBytes = 0);
public void SetMask8(Mask8Image image);
```

`PhotoshopMaskInfo` contains mask dimensions, center position, optional feather
and density, disabled and relative-position flags, and the default color.
There is currently no managed operation that removes an existing mask.

### 8-bit constructors

```csharp
public static PhotoshopGroupLayer CreateGroup(PhotoshopLayerOptions options = default);
public static PhotoshopImageLayer CreateImageRgba8(
    ReadOnlySpan<byte> pixels,
    uint width,
    uint height,
    uint strideBytes = 0,
    PhotoshopLayerOptions options = default);
public static PhotoshopImageLayer CreateImageRgba8(
    Rgba8Image image,
    PhotoshopLayerOptions options = default);
public static PhotoshopImageLayer CreateImageRgb8(
    ReadOnlySpan<byte> pixels,
    uint width,
    uint height,
    uint strideBytes = 0,
    PhotoshopLayerOptions options = default);
public static PhotoshopImageLayer CreateImageRgb8(
    Rgb8Image image,
    PhotoshopLayerOptions options = default);
```

RGB input contains exactly three packed bytes per pixel and creates no alpha
channel. RGBA input uses four packed bytes per pixel with straight alpha. Both
constructors create detached 8-bit layers. Attach them with
`PhotoshopDocument.AddLayer()` or, after attaching a group to an 8-bit
document, `PhotoshopGroupLayer.AddChild()`.

## Specialized layers

### `PhotoshopGroupLayer`

```csharp
public bool Collapsed { get; set; }
public uint ChildCount { get; }
public PhotoshopLayer GetChild(uint index);
public IReadOnlyList<PhotoshopLayer> GetChildren();
public void AddChild(PhotoshopLayer child);
```

For existing documents, prefer `PhotoshopDocument.MoveLayer(child, group)` so
ownership and bit depth are checked against the document.

### `PhotoshopImageLayer`

`PhotoshopImageLayer` adds no methods beyond `PhotoshopLayer`; it identifies
that channel setters are valid. Use the common channel and mask methods.

### `PhotoshopTextLayer`

```csharp
public string Text { get; set; }
public void ReplaceText(string oldText, string newText, bool replaceAll = true);
```

This surface supports content inspection and replacement while preserving the
existing native text structures. It does not yet expose text-layer creation,
character and paragraph range styling, run inspection, orientation, text frame
conversion, writing direction, or text transforms.

### `PhotoshopSmartObjectLayer`

```csharp
public string Hash { get; }
public string Filename { get; }
public string Filepath { get; }

public void Replace(string path, bool linkExternally = false);
public void Move(double xOffset, double yOffset);
public void Rotate(double angleDegrees);
public void Scale(double xFactor, double yFactor);
public void ResetTransform();
public void ResetWarp();
```

Rotation and scaling use the layer center. The current wrapper does not expose
smart-object creation, custom pivot points, arbitrary transformation matrices,
warp inspection or replacement, linkage state, original dimensions, original
full-resolution channels, or linked raw file bytes.

## Enums and value types

- `PhotoshopStatus`
- `PhotoshopBitDepth`
- `PhotoshopColorMode`
- `PhotoshopLayerType`
- `PhotoshopBlendMode`
- `PhotoshopLayerColor`
- `PhotoshopCompression`
- `PhotoshopDocumentInfo`
- `PhotoshopLayerInfo`
- `PhotoshopMaskInfo`
- `PhotoshopLayerOptions`
- `Rgb8Image`
- `Rgba8Image`
- `Mask8Image`

## Original C++ parity audit

This section compares the public C ABI and managed API with the original C++
library. It is based on the upstream
[`LayeredFile`](https://photoshopapi.readthedocs.io/en/latest/code/layeredfile.html),
[`BaseLayer`](https://photoshopapi.readthedocs.io/en/latest/code/layers/baselayer.html),
[`ImageLayer`](https://photoshopapi.readthedocs.io/en/latest/code/layers/image.html),
[`GroupLayer`](https://photoshopapi.readthedocs.io/en/latest/code/layers/group.html),
[`TextLayer`](https://photoshopapi.readthedocs.io/en/latest/concepts/text-layers.html),
and
[`SmartObjectLayer`](https://photoshopapi.readthedocs.io/en/latest/code/layers/smartobject.html)
documentation. "Missing" means the C++ library has a public operation for which
the wrapper currently has no equivalent; it does not mean that ordinary PSD/PSB
read/write is unsupported.

Most managed gaps originate in the C ABI: .NET cannot expose a native operation
that has no stable C entry point. The .NET wrapper covers all material v2 C ABI
features. It intentionally hides opaque native handles, caller-sized error
buffers, and the legacy `photoshopapi_c_document_create` and
`photoshopapi_c_document_write` aliases behind managed objects, exceptions, and
overloads.

| Original C++ API area | C ABI | .NET | Unsupported or different behavior |
| --- | --- | --- | --- |
| PSD/PSB read and write | Supported | Supported | Read auto-detects 8/16/32-bit files. Write consumes the document in both wrappers. Progress callbacks are missing. |
| RGB, CMYK, and Grayscale documents | Supported for existing files | Supported for existing files | The wrappers expose the same practical modes documented upstream. Lab and Multichannel are also unsupported by the original high-level library. |
| Document size and DPI | Supported | Supported | No important gap. |
| Bit depth and color mode | Read-only after creation | Read-only after creation | C++ exposes `colormode(...)`; the wrappers do not expose color-mode mutation/conversion. |
| Document bounding box | Missing | Missing | C++ `bbox()` is not wrapped. A caller can derive a box by traversing layer geometry. |
| Root layer collection | Partial | Partial | Traversal and add/remove/move are supported. Replacing the entire root `layers()` vector is missing. |
| Flattened layers and ordering | Missing | Missing | C++ `flat_layers()` and `LayerOrder` are not exposed. Callers must recursively traverse groups. |
| Document introspection | Partial | Partial | `num_channels()` and `is_layer_in_file()` are missing. |
| ICC profile | Supported | Supported | Raw profile bytes can be read, replaced, or cleared. |
| Global compression | Supported | Supported | `set_compression()` is exposed. |
| Per-layer/channel/mask compression | Missing | Missing | C++ per-layer write compression, compressed channel objects, and mask compression are not exposed. |
| Common layer attributes | Supported | Supported | Name, blend mode, label color, geometry, opacity, fill, visibility, locking, and clipping are exposed. |
| Pixel masks | Partial | Partial | Mask data, geometry, density, feather, default color, relative position, and disabled state are exposed. Mask removal/extraction, inversion controls, and mask-specific compression are missing. |
| Image channels on existing layers | Supported | Supported | Typed per-channel get/set works for 8/16/32-bit layers. Whole-image map get/set, channel views, and bulk dimension-changing replacement are missing. |
| New image layers | 8-bit RGB/RGBA only | 8-bit RGB/RGBA only | Generic channel-map constructors and 16/32-bit constructors are missing. A new 16/32-bit document can be declared but cannot currently be populated from scratch through these wrappers. Existing 16/32-bit layers remain readable and editable. |
| New group layers | 8-bit only | 8-bit only | C++ group constructors for all bit depths are not wrapped. |
| Group hierarchy | Mostly supported | Mostly supported | Child traversal, collapsed state, add, document remove, and move are exposed. Replacing the entire child vector and direct group remove-by-index/name overloads are missing. |
| Text content | Supported | Supported | Read, set, replace, and document text-cache invalidation are exposed. |
| Text creation and typography | Missing | Missing | No text-layer constructor; character style ranges/runs, paragraph styles, fonts, colors, orientation, point/box conversion, writing direction, and text transforms are not exposed. |
| Smart-object identity and basic transforms | Supported | Supported | Hash, filename/path, replace, center-based move/rotate/scale, reset transform, and reset warp are exposed. |
| Smart-object creation and linkage | Missing | Missing | No create-from-file constructor, `linked_externally()` state, `set_linkage()`, or linkage type enum. |
| Smart-object extraction | Missing | Missing | Original dimensions, original channel/image data, and raw linked bytes are not exposed. |
| Smart-object advanced transforms and warp | Missing | Missing | Custom-pivot rotate/scale, arbitrary transform matrices, warp get/set, and warp mesh data are not exposed. |
| Shape, adjustment, artboard, and section layers | Generic access only | Generic access only | These layer kinds are recognized and preserved, and common attributes/channels remain available, but there are no dedicated typed wrapper APIs. |
| Linked-layer document data | Missing | Missing | C++ `linked_layers()` and its raw data, image data, resampling, and interpolation controls are not exposed. Native deduplication remains internal. |
| Unparsed blocks | Missing | Missing | C++ `unparsed_blocks()` inspection is not exposed. |
| Progress reporting | Missing | Missing | C++ read/write progress callbacks are not exposed. |
| Additional C++ enums | Partial | Partial | Channel IDs use integers. `LayerOrder`, linked-layer type, interpolation, text, and warp enums have no wrapper equivalents because their related operations are absent. |

### Intentionally outside the stable wrappers

The following C++ implementation-facing APIs are not supported by either the C
ABI or .NET and should not be treated as ordinary high-level parity gaps:

- raw `PhotoshopFile`, `File`, and `FileSection` parser/writer objects;
- file headers, image resources, tagged blocks, and other serialization records;
- Eigen geometry objects and internal transform representations;
- `compressed::channel` ownership, internal image buffers, render/OIIO helpers,
  logger state, and profiling internals.

These types expose C++ templates, ownership, library-specific containers, or
implementation details that do not have a stable cross-language ABI. They
should remain private unless a separately versioned low-level C API is designed.

### Recommended parity work

1. Add document-aware generic group and image-layer constructors for 8-, 16-,
   and 32-bit samples, including channel maps. This closes the largest general
   document-creation gap.
2. Add text-layer creation and the high-level range styling APIs before
   exposing lower-level text runs. This follows the upstream recommended usage.
3. Add smart-object creation, linkage state, original dimensions/channels,
   custom-pivot transforms, arbitrary transforms, and a stable warp data model.
4. Add mask removal, per-layer compression, progress callbacks, and flattened
   traversal as smaller ABI extensions.

The C ABI should be extended first for each item; the .NET wrapper can then add
the corresponding managed model without binding directly to unstable C++
templates or ownership rules.

## Native deployment

For the validated static Windows x64 build, package only
`PhotoshopAPI.C.dll` under `runtimes/win-x64/native`. Its direct dependencies
are Windows system libraries (`bcrypt.dll`, `KERNEL32.dll`, and `ADVAPI32.dll`).
See `Photoshop.NET.Tests/README.md` for the build and dependency probe.
