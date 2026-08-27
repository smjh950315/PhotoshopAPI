using Photoshop.NET;
using System.Runtime.InteropServices;

Environment.Exit(RunSmokeTest(args));

static int RunSmokeTest(string[] inputDocuments)
{
    try
    {
        const uint width = 2;
        const uint height = 2;
        var runtimeDirectory = Path.Combine(AppContext.BaseDirectory, "runtimes", "win-x64", "native");
        var requiredFiles = new[]
        {
            "PhotoshopAPI.C.dll"
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

            using var loaded = PhotoshopDocument.Read(outputPath);
            var documentInfo = loaded.Info;
            if (documentInfo.Width != width || documentInfo.Height != height ||
                documentInfo.BitDepth != PhotoshopBitDepth.Bit8 || documentInfo.RootLayerCount != 1)
            {
                throw new InvalidOperationException("Read-back document metadata did not match the written PSD.");
            }

            using var loadedRoot = loaded.GetRootLayer(0);
            if (loadedRoot is not PhotoshopGroupLayer loadedGroup || loadedGroup.Name != "Test Group" || loadedGroup.ChildCount != 1)
            {
                throw new InvalidOperationException("The root group hierarchy was not preserved.");
            }

            using var loadedChild = loadedGroup.GetChild(0);
            if (loadedChild is not PhotoshopImageLayer || loadedChild.Name != "Test Image")
            {
                throw new InvalidOperationException("The image child was not exposed as a typed image layer.");
            }

            var channelIndices = loadedChild.GetChannelIndices();
            if (!channelIndices.Contains(0) || !channelIndices.Contains(1) || !channelIndices.Contains(2))
            {
                throw new InvalidOperationException("The RGB channels were not available after read-back.");
            }
            if (loadedChild.GetChannel8(0).Length != width * height || loadedChild.GetMaskBytes().Length != width * height)
            {
                throw new InvalidOperationException("Channel or mask data had an unexpected size after read-back.");
            }
        }
        finally
        {
            File.Delete(outputPath);
        }

        foreach (var inputDocument in inputDocuments)
        {
            using var loaded = PhotoshopDocument.Read(inputDocument);
            var info = loaded.Info;
            if (info.RootLayerCount == 0)
            {
                throw new InvalidOperationException($"Input document has no readable root layers: {inputDocument}");
            }
            for (uint index = 0; index < info.RootLayerCount; ++index)
            {
                using var rootLayer = loaded.GetRootLayer(index);
                ProbeSpecializedLayer(rootLayer);
            }
            Console.WriteLine($"Read {info.BitDepth} {info.ColorMode} document: {inputDocument}");
        }

        Console.WriteLine("Photoshop.NET create, write, read, hierarchy, channel, and mask smoke test passed.");
        return 0;
    }
    catch (Exception exception)
    {
        Console.Error.WriteLine(exception);
        return 1;
    }
}

static void ProbeSpecializedLayer(PhotoshopLayer layer)
{
    switch (layer)
    {
        case PhotoshopGroupLayer group:
            for (uint index = 0; index < group.ChildCount; ++index)
            {
                using var child = group.GetChild(index);
                ProbeSpecializedLayer(child);
            }
            break;
        case PhotoshopTextLayer text:
            _ = text.Text;
            break;
        case PhotoshopSmartObjectLayer smartObject:
            _ = smartObject.Hash;
            _ = smartObject.Filename;
            _ = smartObject.Filepath;
            break;
    }
}
