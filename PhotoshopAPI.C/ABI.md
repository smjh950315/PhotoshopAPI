# PhotoshopAPI C ABI design

## Goals

The C ABI exposes the high-level `LayeredFile` model used by the C++ and Python
APIs while remaining safe to consume from .NET and other foreign-function
interfaces. ABI version 2 keeps the original creation and write functions and
adds PSD/PSB reading, automatic bit-depth handling, layer traversal, common
layer editing, channel and mask access, text editing, and smart-object
operations. Image-layer construction accepts packed RGB8 data without alpha or
straight-alpha RGBA8 data.

## Ownership

- `photoshopapi_c_document*` and `photoshopapi_c_layer*` are opaque owning
  handles. Every handle returned through an `out_*` parameter must be released
  with its matching destroy function.
- A layer handle owns a shared reference to its C++ layer. Destroying a layer
  handle never removes the layer from its document.
- Document, group, and layer hierarchy functions retain the C++ layer through
  the document. Handles returned for existing layers can be destroyed in any
  order.
- Writing consumes the document's layered-file value, matching the C++ API.
  The document handle remains valid for error retrieval and destruction, but
  cannot be used for further document operations.

## Strings and buffers

- Input paths and strings are UTF-8 and NUL terminated.
- Variable-size output uses a two-call contract. Pass `NULL` and capacity zero
  to query the required size, allocate a caller-owned buffer, then call again.
- Required string sizes include the trailing NUL byte. Binary and pixel buffer
  sizes are reported in bytes.
- Channel and mask samples use the document bit depth: `uint8_t` for 8-bit,
  `uint16_t` for 16-bit, and IEEE 754 `float` for 32-bit documents.
- `photoshopapi_c_rgb8_view` contains packed red, green, and blue bytes;
  `photoshopapi_c_rgba8_view` additionally contains a straight alpha byte.
  A zero stride selects tightly packed rows (width times three or four bytes).

## Threading

Handles do not provide internal synchronization. Separate documents may be
used concurrently, but callers must serialize access to the same document or
layer handle.

## Compatibility

New functions may be added without changing existing layouts. Existing enum
values and public structure fields are stable for the lifetime of ABI version
2. A future incompatible layout or ownership change requires a new ABI
version.
