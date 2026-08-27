using System.Runtime.InteropServices;
using System.Text;

namespace Photoshop.NET;

/// <summary>
/// P/Invoke declarations for the PhotoshopAPI C ABI.
/// </summary>
internal static class NativeMethods
{
    private const string LibraryName = "PhotoshopAPI.C";

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true, EntryPoint = "photoshopapi_c_get_abi_version")]
    internal static extern uint GetAbiVersion();

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true, EntryPoint = "photoshopapi_c_document_create")]
    internal static extern PhotoshopStatus CreateDocument(uint width, uint height, out nint document);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true, EntryPoint = "photoshopapi_c_document_destroy")]
    internal static extern void DestroyDocument(nint document);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true, EntryPoint = "photoshopapi_c_group_layer_create")]
    internal static extern PhotoshopStatus CreateGroupLayer(
        ref NativeLayerOptionsData options,
        out nint layer);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true, EntryPoint = "photoshopapi_c_image_layer_create_rgba8")]
    internal static extern PhotoshopStatus CreateImageLayer(
        ref NativeRgba8View source,
        ref NativeLayerOptionsData options,
        out nint layer);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true, EntryPoint = "photoshopapi_c_layer_destroy")]
    internal static extern void DestroyLayer(nint layer);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true, EntryPoint = "photoshopapi_c_document_add_layer")]
    internal static extern PhotoshopStatus DocumentAddLayer(nint document, nint layer);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true, EntryPoint = "photoshopapi_c_group_add_layer")]
    internal static extern PhotoshopStatus GroupAddLayer(nint group, nint child);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true, EntryPoint = "photoshopapi_c_layer_set_mask8")]
    internal static extern PhotoshopStatus SetMask(nint layer, ref NativeMask8View mask);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true, EntryPoint = "photoshopapi_c_document_write")]
    internal static extern PhotoshopStatus DocumentWrite(nint document, nint utf8Path);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true, EntryPoint = "photoshopapi_c_document_get_last_error")]
    private static extern PhotoshopStatus GetDocumentErrorNative(
        nint document,
        nint buffer,
        uint bufferCapacity,
        out uint requiredBufferSize);

    [DllImport(LibraryName, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true, EntryPoint = "photoshopapi_c_layer_get_last_error")]
    private static extern PhotoshopStatus GetLayerErrorNative(
        nint layer,
        nint buffer,
        uint bufferCapacity,
        out uint requiredBufferSize);

    internal static string GetDocumentError(nint document) => ReadError(
        (nint buffer, uint capacity, out uint required) =>
            GetDocumentErrorNative(document, buffer, capacity, out required));

    internal static string GetLayerError(nint layer) => ReadError(
        (nint buffer, uint capacity, out uint required) =>
            GetLayerErrorNative(layer, buffer, capacity, out required));

    private static string ReadError(ErrorReader reader)
    {
        var status = reader(nint.Zero, 0, out var required);
        if (status != PhotoshopStatus.Success || required == 0 || required > int.MaxValue)
        {
            return string.Empty;
        }

        var buffer = new byte[(int)required];
        var pinned = GCHandle.Alloc(buffer, GCHandleType.Pinned);
        try
        {
            status = reader(pinned.AddrOfPinnedObject(), required, out var actualRequired);
            if (status != PhotoshopStatus.Success || actualRequired > required)
            {
                return string.Empty;
            }
            return Encoding.UTF8.GetString(buffer, 0, Math.Max(0, (int)actualRequired - 1));
        }
        finally
        {
            pinned.Free();
        }
    }

    private delegate PhotoshopStatus ErrorReader(nint buffer, uint capacity, out uint required);
}
