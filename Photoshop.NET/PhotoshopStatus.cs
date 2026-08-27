namespace Photoshop.NET;

/// <summary>
/// Status codes returned by the native PhotoshopAPI C ABI.
/// </summary>
public enum PhotoshopStatus
{
    /// <summary>The operation completed successfully.</summary>
    Success = 0,
    /// <summary>An argument was invalid.</summary>
    InvalidArgument = 1,
    /// <summary>The supplied dimensions were invalid.</summary>
    DimensionsInvalid = 2,
    /// <summary>The supplied dimensions did not match.</summary>
    DimensionsMismatch = 3,
    /// <summary>A native allocation failed.</summary>
    AllocationFailed = 4,
    /// <summary>An ownership or lifetime rule was violated.</summary>
    OwnershipError = 5,
    /// <summary>An input/output operation failed.</summary>
    IoError = 6,
    /// <summary>Writing the document failed.</summary>
    WriteFailed = 7,
    /// <summary>The requested operation is not supported.</summary>
    NotSupported = 8,
    /// <summary>A supplied buffer was too small.</summary>
    BufferTooSmall = 9,
    /// <summary>The native object has a different layer kind or bit depth.</summary>
    TypeMismatch = 10,
    /// <summary>Reading the Photoshop document failed.</summary>
    ReadFailed = 11,
    /// <summary>An unexpected native failure occurred.</summary>
    InternalError = 12
}
