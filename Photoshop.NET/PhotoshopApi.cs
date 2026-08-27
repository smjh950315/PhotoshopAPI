namespace Photoshop.NET;

/// <summary>
/// Provides ABI compatibility checks and common native error handling.
/// </summary>
public static class PhotoshopApi
{
    /// <summary>
    /// Gets the ABI version expected by this managed wrapper.
    /// </summary>
    public const uint AbiVersion = 2;

    /// <summary>
    /// Gets the ABI version reported by the loaded native library.
    /// </summary>
    public static uint NativeAbiVersion => NativeMethods.GetAbiVersion();

    /// <summary>
    /// Verifies that the loaded native library matches this wrapper's ABI.
    /// </summary>
    /// <exception cref="PhotoshopApiException">Thrown when the versions differ.</exception>
    public static void EnsureCompatible()
    {
        var version = NativeAbiVersion;
        if (version != AbiVersion)
        {
            throw new PhotoshopApiException(
                PhotoshopStatus.NotSupported,
                $"PhotoshopAPI.C ABI version {version} is not supported; expected {AbiVersion}.");
        }
    }

    internal static void ThrowIfFailed(PhotoshopStatus status, string? nativeError = null)
    {
        if (status != PhotoshopStatus.Success)
        {
            throw new PhotoshopApiException(status, string.IsNullOrWhiteSpace(nativeError)
                ? GetDefaultMessage(status)
                : nativeError);
        }
    }

    internal static void ValidateImageBuffer(
        ReadOnlySpan<byte> pixels,
        uint width,
        uint height,
        uint strideBytes,
        uint bytesPerPixel,
        string parameterName)
    {
        if (width == 0 || height == 0)
        {
            throw new ArgumentOutOfRangeException(parameterName, "Width and height must be non-zero.");
        }

        var minimumStride = checked((ulong)width * bytesPerPixel);
        var stride = strideBytes == 0 ? minimumStride : strideBytes;
        if (stride < minimumStride)
        {
            throw new ArgumentException("Stride is smaller than one packed row.", parameterName);
        }

        var requiredBytes = checked((height - 1UL) * stride + minimumStride);
        if (requiredBytes > (ulong)pixels.Length)
        {
            throw new ArgumentException(
                $"The buffer contains {pixels.Length} bytes, but at least {requiredBytes} are required.",
                parameterName);
        }
    }

    private static string GetDefaultMessage(PhotoshopStatus status) => status switch
    {
        PhotoshopStatus.InvalidArgument => "The native API rejected an argument.",
        PhotoshopStatus.DimensionsInvalid => "The supplied dimensions are invalid.",
        PhotoshopStatus.DimensionsMismatch => "The supplied dimensions do not match.",
        PhotoshopStatus.AllocationFailed => "The native API could not allocate memory.",
        PhotoshopStatus.OwnershipError => "The native object is no longer usable or belongs to another document.",
        PhotoshopStatus.IoError => "The native API could not access the requested file.",
        PhotoshopStatus.WriteFailed => "The native API failed to write the document.",
        PhotoshopStatus.NotSupported => "The requested operation is not supported.",
        PhotoshopStatus.BufferTooSmall => "The supplied buffer is too small.",
        PhotoshopStatus.TypeMismatch => "The layer type or bit depth does not support this operation.",
        PhotoshopStatus.ReadFailed => "The native API failed to read the Photoshop document.",
        PhotoshopStatus.InternalError => "The native API encountered an unexpected error.",
        _ => $"PhotoshopAPI.C returned status {(int)status}."
    };
}
