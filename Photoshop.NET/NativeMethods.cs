using System.Runtime.InteropServices;
using System.Text;

namespace Photoshop.NET;

/// <summary>P/Invoke declarations and two-call buffer helpers for PhotoshopAPI.C.</summary>
internal static class NativeMethods
{
    private const string LibraryName = "PhotoshopAPI.C";
    private const CallingConvention Convention = CallingConvention.Cdecl;

    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_get_abi_version")]
    internal static extern uint GetAbiVersion();
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_document_create")]
    internal static extern PhotoshopStatus CreateDocument(uint width, uint height, out nint document);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_document_create_ex")]
    internal static extern PhotoshopStatus CreateDocumentEx(PhotoshopBitDepth depth, PhotoshopColorMode mode, ulong width, ulong height, out nint document);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_document_read")]
    internal static extern PhotoshopStatus ReadDocument(nint path, out nint document);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_document_destroy")]
    internal static extern void DestroyDocument(nint document);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_document_get_info")]
    internal static extern PhotoshopStatus GetDocumentInfo(nint document, out NativeDocumentInfo info);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_document_set_size")]
    internal static extern PhotoshopStatus SetDocumentSize(nint document, ulong width, ulong height);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_document_set_dpi")]
    internal static extern PhotoshopStatus SetDocumentDpi(nint document, float dpi);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_document_set_compression")]
    internal static extern PhotoshopStatus SetDocumentCompression(nint document, PhotoshopCompression compression);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_document_set_merged_rgb8")]
    internal static extern PhotoshopStatus SetDocumentMergedRgb8(nint document, ref NativeRgb8View source);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_document_invalidate_text_cache")]
    internal static extern PhotoshopStatus InvalidateTextCache(nint document);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_document_get_icc_profile")]
    internal static extern PhotoshopStatus GetIccProfile(nint document, nint buffer, ulong capacity, out ulong required);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_document_set_icc_profile")]
    internal static extern PhotoshopStatus SetIccProfile(nint document, nint data, ulong size);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_document_get_root_layer")]
    internal static extern PhotoshopStatus GetRootLayer(nint document, uint index, out nint layer);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_document_find_layer")]
    internal static extern PhotoshopStatus FindLayer(nint document, nint path, out nint layer);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_document_add_layer")]
    internal static extern PhotoshopStatus DocumentAddLayer(nint document, nint layer);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_document_remove_layer")]
    internal static extern PhotoshopStatus DocumentRemoveLayer(nint document, nint layer);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_document_move_layer")]
    internal static extern PhotoshopStatus DocumentMoveLayer(nint document, nint layer, nint parent);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_document_write_ex")]
    internal static extern PhotoshopStatus DocumentWriteEx(nint document, nint path, byte overwrite);

    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_group_layer_create")]
    internal static extern PhotoshopStatus CreateGroupLayer(ref NativeLayerOptionsData options, out nint layer);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_image_layer_create_rgba8")]
    internal static extern PhotoshopStatus CreateRgba8ImageLayer(ref NativeRgba8View source, ref NativeLayerOptionsData options, out nint layer);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_image_layer_create_rgb8")]
    internal static extern PhotoshopStatus CreateRgb8ImageLayer(ref NativeRgb8View source, ref NativeLayerOptionsData options, out nint layer);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_layer_destroy")]
    internal static extern void DestroyLayer(nint layer);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_layer_get_info")]
    internal static extern PhotoshopStatus GetLayerInfo(nint layer, out NativeLayerInfo info);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_layer_set_info")]
    internal static extern PhotoshopStatus SetLayerInfo(nint layer, ref NativeLayerInfo info);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_layer_get_name")]
    internal static extern PhotoshopStatus GetLayerName(nint layer, nint buffer, uint capacity, out uint required);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_layer_set_name")]
    internal static extern PhotoshopStatus SetLayerName(nint layer, nint name);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_layer_get_child_count")]
    internal static extern PhotoshopStatus GetChildCount(nint layer, out uint count);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_layer_get_child")]
    internal static extern PhotoshopStatus GetChild(nint layer, uint index, out nint child);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_group_get_collapsed")]
    internal static extern PhotoshopStatus GetCollapsed(nint layer, out byte collapsed);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_group_set_collapsed")]
    internal static extern PhotoshopStatus SetCollapsed(nint layer, byte collapsed);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_group_add_layer")]
    internal static extern PhotoshopStatus GroupAddLayer(nint group, nint child);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_layer_set_mask8")]
    internal static extern PhotoshopStatus SetMask8(nint layer, ref NativeMask8View mask);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_layer_get_channel_indices")]
    internal static extern PhotoshopStatus GetChannelIndices(nint layer, nint buffer, uint capacity, out uint required);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_layer_get_channel_data")]
    internal static extern PhotoshopStatus GetChannelData(nint layer, int channel, nint buffer, ulong capacity, out ulong required);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_layer_set_channel_data")]
    internal static extern PhotoshopStatus SetChannelData(nint layer, int channel, nint data, ulong size);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_layer_get_mask_info")]
    internal static extern PhotoshopStatus GetMaskInfo(nint layer, out NativeMaskInfo info);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_layer_set_mask_info")]
    internal static extern PhotoshopStatus SetMaskInfo(nint layer, ref NativeMaskInfo info);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_layer_get_mask_data")]
    internal static extern PhotoshopStatus GetMaskData(nint layer, nint buffer, ulong capacity, out ulong required);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_layer_set_mask_data")]
    internal static extern PhotoshopStatus SetMaskData(nint layer, nint data, ulong size, ulong width, ulong height);

    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_text_layer_get_text")]
    internal static extern PhotoshopStatus GetText(nint layer, nint buffer, uint capacity, out uint required);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_text_layer_set_text")]
    internal static extern PhotoshopStatus SetText(nint layer, nint text);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_text_layer_replace_text")]
    internal static extern PhotoshopStatus ReplaceText(nint layer, nint oldText, nint newText, byte replaceAll);

    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_smart_object_get_hash")]
    internal static extern PhotoshopStatus GetSmartHash(nint layer, nint buffer, uint capacity, out uint required);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_smart_object_get_filename")]
    internal static extern PhotoshopStatus GetSmartFilename(nint layer, nint buffer, uint capacity, out uint required);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_smart_object_get_filepath")]
    internal static extern PhotoshopStatus GetSmartFilepath(nint layer, nint buffer, uint capacity, out uint required);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_smart_object_replace")]
    internal static extern PhotoshopStatus ReplaceSmartObject(nint layer, nint path, byte external);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_smart_object_move")]
    internal static extern PhotoshopStatus MoveSmartObject(nint layer, double x, double y);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_smart_object_rotate")]
    internal static extern PhotoshopStatus RotateSmartObject(nint layer, double angle);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_smart_object_scale")]
    internal static extern PhotoshopStatus ScaleSmartObject(nint layer, double x, double y);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_smart_object_reset_transform")]
    internal static extern PhotoshopStatus ResetSmartTransform(nint layer);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_smart_object_reset_warp")]
    internal static extern PhotoshopStatus ResetSmartWarp(nint layer);

    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_document_get_last_error")]
    private static extern PhotoshopStatus GetDocumentErrorNative(nint document, nint buffer, uint capacity, out uint required);
    [DllImport(LibraryName, CallingConvention = Convention, ExactSpelling = true, EntryPoint = "photoshopapi_c_layer_get_last_error")]
    private static extern PhotoshopStatus GetLayerErrorNative(nint layer, nint buffer, uint capacity, out uint required);

    internal static string GetDocumentError(nint document) => ReadString((nint b, uint c, out uint r) => GetDocumentErrorNative(document, b, c, out r));
    internal static string GetLayerError(nint layer) => ReadString((nint b, uint c, out uint r) => GetLayerErrorNative(layer, b, c, out r));

    internal static string ReadString(StringReader reader)
    {
        var status = reader(nint.Zero, 0, out var required);
        PhotoshopApi.ThrowIfFailed(status);
        if (required == 0 || required > int.MaxValue) return string.Empty;
        var data = new byte[(int)required];
        using var pinned = new PinnedBuffer(data);
        status = reader(pinned.Pointer, required, out var actual);
        PhotoshopApi.ThrowIfFailed(status);
        return Encoding.UTF8.GetString(data, 0, Math.Max(0, checked((int)actual) - 1));
    }

    internal static byte[] ReadBytes(ByteReader reader)
    {
        var status = reader(nint.Zero, 0, out var required);
        PhotoshopApi.ThrowIfFailed(status);
        if (required == 0) return [];
        if (required > int.MaxValue) throw new OverflowException("Native buffer is too large for a managed array.");
        var data = new byte[(int)required];
        using var pinned = new PinnedBuffer(data);
        status = reader(pinned.Pointer, required, out var actual);
        PhotoshopApi.ThrowIfFailed(status);
        if (actual != required) Array.Resize(ref data, checked((int)actual));
        return data;
    }

    internal delegate PhotoshopStatus StringReader(nint buffer, uint capacity, out uint required);
    internal delegate PhotoshopStatus ByteReader(nint buffer, ulong capacity, out ulong required);
}
