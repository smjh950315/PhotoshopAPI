namespace Photoshop.NET;

/// <summary>
/// Represents an error reported by the PhotoshopAPI C ABI.
/// </summary>
public sealed class PhotoshopApiException : Exception
{
    /// <summary>
    /// Creates an exception for a native status code.
    /// </summary>
    /// <param name="status">The status returned by the native API.</param>
    /// <param name="message">A human-readable description of the failure.</param>
    public PhotoshopApiException(PhotoshopStatus status, string message)
        : base(message)
    {
        Status = status;
    }

    /// <summary>
    /// Gets the native status code associated with the exception.
    /// </summary>
    public PhotoshopStatus Status { get; }
}
