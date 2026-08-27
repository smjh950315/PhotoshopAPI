using Photoshop.NET;
using System.Runtime.InteropServices;

Environment.Exit(RunSmokeTest());

static int RunSmokeTest()
{
    try
    {
        const uint width = 2;
        const uint height = 2;
        var runtimeDirectory = Path.Combine(AppContext.BaseDirectory, "runtimes", "win-x64", "native");
        var requiredFiles = new[]
        {
            "PhotoshopAPI.C.dll",
            "deflate.dll"
        };

        foreach (var fileName in requiredFiles)
        {
            var path = Path.Combine(runtimeDirectory, fileName);
            if (!File.Exists(path))
            {
                throw new InvalidOperationException($"Required native runtime file was not copied: {path}");
            }
        }

        var runtimeLibrary = NativeLibrary.Load(Path.Combine(runtimeDirectory, "PhotoshopAPI.C.dll"));
        NativeLibrary.Free(runtimeLibrary);

        PhotoshopApi.EnsureCompatible();

        using var document = PhotoshopDocument.Create(width, height);
        using var group = PhotoshopLayer.CreateGroup(new PhotoshopLayerOptions(Name: "Test Group"));
        using var image = PhotoshopLayer.CreateImageRgba8(
            new byte[]
            {
                255, 0, 0, 255,
                0, 255, 0, 255,
                0, 0, 255, 255,
                255, 255, 255, 255
            },
            width,
            height,
            options: new PhotoshopLayerOptions(Name: "Test Image"));

        image.SetMask8(new byte[] { 255, 192, 128, 0 }, width, height);
        document.AddLayer(group);
        group.AddChild(image);

        var outputPath = Path.Combine(Path.GetTempPath(), $"Photoshop.NET.Tests-{Guid.NewGuid():N}.psd");
        try
        {
            document.Write(outputPath);
            if (!File.Exists(outputPath) || new FileInfo(outputPath).Length == 0)
            {
                throw new InvalidOperationException("The managed wrapper did not write the test PSD.");
            }
        }
        finally
        {
            File.Delete(outputPath);
        }

        Console.WriteLine("Photoshop.NET native load and wrapper smoke test passed.");
        return 0;
    }
    catch (Exception exception)
    {
        Console.Error.WriteLine(exception);
        return 1;
    }
}
