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
        RunProductionMatrix();

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
        using var rgbImage = PhotoshopLayer.CreateImageRgb8(
            new Rgb8Image(
                new byte[]
                {
                    255, 255, 0,
                    0, 255, 255,
                    255, 0, 255,
                    0, 0, 0
                },
                width,
                height),
            options: new PhotoshopLayerOptions(Name: "Test RGB Image"));

        image.SetMask8(new byte[] { 255, 192, 128, 0 }, width, height);
        document.AddLayer(group);
        group.AddChild(image);
        group.AddChild(rgbImage);

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
            if (loadedRoot is not PhotoshopGroupLayer loadedGroup || loadedGroup.Name != "Test Group" || loadedGroup.ChildCount != 2)
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

            using var loadedRgb = loaded.FindLayer("Test Group/Test RGB Image");
            if (loadedRgb is not PhotoshopImageLayer)
            {
                throw new InvalidOperationException("The RGB image was not exposed as a typed image layer.");
            }
            var rgbChannelIndices = loadedRgb.GetChannelIndices();
            if (!rgbChannelIndices.Contains(0) || !rgbChannelIndices.Contains(1) ||
                !rgbChannelIndices.Contains(2) || rgbChannelIndices.Contains(-1))
            {
                throw new InvalidOperationException("The RGB image did not preserve exactly the three color channels.");
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

        Console.WriteLine("Photoshop.NET RGB/RGBA create, write, read, hierarchy, channel, and mask smoke test passed.");
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

static void RunProductionMatrix()
{
    foreach (var extension in new[] { ".psd", ".psb" })
    {
        foreach (var compression in Enum.GetValues<PhotoshopCompression>())
        {
            RunRgb8RoundTrip(extension, compression);
        }
    }

    foreach (var depth in new[] { PhotoshopBitDepth.Bit16, PhotoshopBitDepth.Bit32 })
    {
        foreach (var extension in new[] { ".psd", ".psb" })
        {
            RunMetadataRoundTrip(depth, extension);
        }
    }

    Console.WriteLine("Photoshop.NET PSD/PSB and compression production matrix passed.");
}

static void RunRgb8RoundTrip(string extension, PhotoshopCompression compression)
{
    const uint width = 2;
    const uint height = 2;
    var rgbaPixels = new byte[]
    {
        255, 0, 0, 255,
        0, 255, 0, 192,
        0, 0, 255, 128,
        255, 255, 255, 0
    };
    var rgbPixels = new byte[]
    {
        255, 255, 0,
        0, 255, 255,
        255, 0, 255,
        0, 0, 0
    };
    var maskPixels = new byte[] { 255, 192, 128, 0 };
    var mergedPixels = new byte[]
    {
        64, 32, 16,
        80, 48, 24,
        96, 64, 32,
        112, 80, 40
    };

    using var document = PhotoshopDocument.Create(width, height);
    document.SetCompression(compression);
    using var group = PhotoshopLayer.CreateGroup(new PhotoshopLayerOptions(Name: "Test Group"));
    using var image = PhotoshopLayer.CreateImageRgba8(
        rgbaPixels,
        width,
        height,
        options: new PhotoshopLayerOptions(Name: "Test Image"));
    using var rgbImage = PhotoshopLayer.CreateImageRgb8(
        rgbPixels,
        width,
        height,
        options: new PhotoshopLayerOptions(Name: "Test RGB Image"));

    image.SetMask8(maskPixels, width, height);
    document.AddLayer(group);
    group.AddChild(image);
    group.AddChild(rgbImage);
    document.SetMergedImage(new Rgb8Image(mergedPixels, width, height));

    var outputPath = Path.Combine(Path.GetTempPath(), $"Photoshop.NET.Tests-{Guid.NewGuid():N}{extension}");
    try
    {
        document.Write(outputPath);
        Require(File.Exists(outputPath) && new FileInfo(outputPath).Length > 0,
            $"The managed wrapper did not write a non-empty {extension} using {compression}.");

        using var loaded = PhotoshopDocument.Read(outputPath);
        var documentInfo = loaded.Info;
        Require(documentInfo.Width == width && documentInfo.Height == height &&
                documentInfo.BitDepth == PhotoshopBitDepth.Bit8 &&
                documentInfo.ColorMode == PhotoshopColorMode.Rgb &&
                documentInfo.RootLayerCount == 1,
            $"Read-back {extension} metadata did not match the written document using {compression}.");

        using var loadedRoot = loaded.GetRootLayer(0);
        Require(loadedRoot is PhotoshopGroupLayer &&
                loadedRoot.Name == "Test Group" && ((PhotoshopGroupLayer)loadedRoot).ChildCount == 2,
            $"The root group hierarchy was not preserved for {extension} using {compression}.");

        using var loadedChild = ((PhotoshopGroupLayer)loadedRoot).GetChild(0);
        Require(loadedChild is PhotoshopImageLayer && loadedChild.Name == "Test Image",
            $"The RGBA child was not exposed as a typed image layer for {extension} using {compression}.");

        var channelIndices = loadedChild.GetChannelIndices();
        Require(channelIndices.Contains(0) && channelIndices.Contains(1) &&
                channelIndices.Contains(2) && channelIndices.Contains(-1),
            $"The RGBA channels were not available for {extension} using {compression}.");
        AssertBytesEqual(GetPackedChannel(rgbaPixels, 4, 0), loadedChild.GetChannel8(0), "red channel", extension, compression);
        AssertBytesEqual(GetPackedChannel(rgbaPixels, 4, 1), loadedChild.GetChannel8(1), "green channel", extension, compression);
        AssertBytesEqual(GetPackedChannel(rgbaPixels, 4, 2), loadedChild.GetChannel8(2), "blue channel", extension, compression);
        AssertBytesEqual(GetPackedChannel(rgbaPixels, 4, 3), loadedChild.GetChannel8(-1), "alpha channel", extension, compression);
        AssertBytesEqual(maskPixels, loadedChild.GetMaskBytes(), "layer mask", extension, compression);

        var maskInfo = loadedChild.GetMaskInfo();
        Require(maskInfo.HasMask && maskInfo.Width == width && maskInfo.Height == height,
            $"The layer mask metadata was not preserved for {extension} using {compression}.");

        using var loadedRgb = loaded.FindLayer("Test Group/Test RGB Image");
        Require(loadedRgb is PhotoshopImageLayer && loadedRgb.Name == "Test RGB Image",
            $"The RGB image was not found through its hierarchy path for {extension} using {compression}.");
        var rgbChannelIndices = loadedRgb.GetChannelIndices();
        Require(rgbChannelIndices.Contains(0) && rgbChannelIndices.Contains(1) &&
                rgbChannelIndices.Contains(2) && !rgbChannelIndices.Contains(-1),
            $"The RGB image did not preserve exactly three color channels for {extension} using {compression}.");
        AssertBytesEqual(GetPackedChannel(rgbPixels, 3, 0), loadedRgb.GetChannel8(0), "RGB red channel", extension, compression);
        AssertBytesEqual(GetPackedChannel(rgbPixels, 3, 1), loadedRgb.GetChannel8(1), "RGB green channel", extension, compression);
        AssertBytesEqual(GetPackedChannel(rgbPixels, 3, 2), loadedRgb.GetChannel8(2), "RGB blue channel", extension, compression);

        Console.WriteLine($"Passed {extension} RGB8 hierarchy/channel/mask/merged round-trip with {compression}.");
    }
    finally
    {
        File.Delete(outputPath);
    }
}

static void RunMetadataRoundTrip(PhotoshopBitDepth depth, string extension)
{
    var bitDepth = depth == PhotoshopBitDepth.Bit16 ? 16 : 32;
    var inputPath = Path.Combine(
        AppContext.BaseDirectory,
        "fixtures",
        $"SingleLayer_{bitDepth}bit{extension}");
    var outputPath = Path.Combine(Path.GetTempPath(), $"Photoshop.NET.Tests-{Guid.NewGuid():N}{extension}");
    try
    {
        Require(File.Exists(inputPath), $"The {depth} {extension} fixture was not copied to the test output.");

        using var document = PhotoshopDocument.Read(inputPath);
        var sourceInfo = document.Info;
        Require(sourceInfo.BitDepth == depth && sourceInfo.ColorMode == PhotoshopColorMode.Rgb &&
                sourceInfo.RootLayerCount > 0,
            $"The source {extension} fixture is not a layered RGB {depth} document.");

        document.Write(outputPath);
        Require(File.Exists(outputPath) && new FileInfo(outputPath).Length > 0,
            $"The managed wrapper did not write a non-empty {extension} {depth} document.");

        using var loaded = PhotoshopDocument.Read(outputPath);
        var info = loaded.Info;
        Require(info.Width == sourceInfo.Width && info.Height == sourceInfo.Height &&
                info.BitDepth == depth && info.ColorMode == PhotoshopColorMode.Rgb &&
                info.RootLayerCount == sourceInfo.RootLayerCount,
            $"Read-back {extension} metadata did not preserve {depth} samples.");
        Console.WriteLine($"Passed {extension} {depth} layered metadata round-trip.");
    }
    finally
    {
        File.Delete(outputPath);
    }
}

static byte[] GetPackedChannel(byte[] pixels, int channelCount, int channelIndex)
{
    var channel = new byte[pixels.Length / channelCount];
    for (var pixel = 0; pixel < channel.Length; ++pixel)
    {
        channel[pixel] = pixels[pixel * channelCount + channelIndex];
    }
    return channel;
}

static void AssertBytesEqual(byte[] expected, byte[] actual, string description, string extension, PhotoshopCompression compression)
{
    Require(expected.AsSpan().SequenceEqual(actual),
        $"The {description} changed after {extension} {compression} round-trip.");
}

static void Require(bool condition, string message)
{
    if (!condition) throw new InvalidOperationException(message);
}
