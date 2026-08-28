#include <PhotoshopAPI.C/PhotoshopAPI.C.h>

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
constexpr uint32_t kWidth = 256;
constexpr uint32_t kHeight = 192;
constexpr uint32_t kForegroundWidth = 96;
constexpr uint32_t kForegroundHeight = 80;
constexpr uint32_t kForegroundLeft = 80;
constexpr uint32_t kForegroundTop = 56;

struct FixtureImages
{
    std::vector<uint8_t> background = std::vector<uint8_t>(kWidth * kHeight * 3u);
    std::vector<uint8_t> foreground = std::vector<uint8_t>(kForegroundWidth * kForegroundHeight * 4u);
    std::vector<uint8_t> mask = std::vector<uint8_t>(kForegroundWidth * kForegroundHeight, 255u);
    std::vector<uint8_t> merged = std::vector<uint8_t>(kWidth * kHeight * 3u);
};

void check(photoshopapi_c_status status, const char* operation)
{
    if (status != PHOTOSHOPAPI_C_STATUS_SUCCESS)
    {
        throw std::runtime_error(std::string(operation) + " failed with status " + std::to_string(status));
    }
}

FixtureImages make_images()
{
    FixtureImages images;
    for (uint32_t y = 0; y < kHeight; ++y)
    {
        for (uint32_t x = 0; x < kWidth; ++x)
        {
            const auto index = (static_cast<size_t>(y) * kWidth + x) * 3u;
            images.background[index] = static_cast<uint8_t>(32u + x / 4u);
            images.background[index + 1u] = static_cast<uint8_t>(48u + y / 3u);
            images.background[index + 2u] = 96u;
        }
    }

    for (uint32_t y = 0; y < kForegroundHeight; ++y)
    {
        for (uint32_t x = 0; x < kForegroundWidth; ++x)
        {
            const auto index = (static_cast<size_t>(y) * kForegroundWidth + x) * 4u;
            const auto border = x < 5u || y < 5u || x + 5u >= kForegroundWidth || y + 5u >= kForegroundHeight;
            images.foreground[index] = 230u;
            images.foreground[index + 1u] = static_cast<uint8_t>(40u + y);
            images.foreground[index + 2u] = 70u;
            images.foreground[index + 3u] = border ? 120u : 230u;
            images.mask[static_cast<size_t>(y) * kForegroundWidth + x] =
                (x < 12u || y < 12u || x + 12u >= kForegroundWidth || y + 12u >= kForegroundHeight) ? 160u : 255u;
        }
    }

    images.merged = images.background;
    for (uint32_t y = 0; y < kForegroundHeight; ++y)
    {
        for (uint32_t x = 0; x < kForegroundWidth; ++x)
        {
            const auto foreground_index = (static_cast<size_t>(y) * kForegroundWidth + x) * 4u;
            const auto canvas_index = (static_cast<size_t>(y + kForegroundTop) * kWidth + x + kForegroundLeft) * 3u;
            const auto alpha = static_cast<uint32_t>(images.foreground[foreground_index + 3u]) *
                images.mask[static_cast<size_t>(y) * kForegroundWidth + x] / 255u;
            for (uint32_t channel = 0; channel < 3u; ++channel)
            {
                const auto source = images.foreground[foreground_index + channel];
                const auto destination = images.merged[canvas_index + channel];
                images.merged[canvas_index + channel] = static_cast<uint8_t>(
                    (source * alpha + destination * (255u - alpha) + 127u) / 255u);
            }
        }
    }
    return images;
}

void write_fixture(const std::filesystem::path& path, bool grouped, bool masked)
{
    const auto images = make_images();
    photoshopapi_c_document* document = nullptr;
    photoshopapi_c_layer* background = nullptr;
    photoshopapi_c_layer* foreground = nullptr;
    photoshopapi_c_layer* group = nullptr;
    const photoshopapi_c_layer_options background_options{"Background", 0, 0, 1.0f, 1, 0, {0, 0}};
    const photoshopapi_c_layer_options foreground_options{"Foreground", static_cast<int32_t>(kForegroundLeft), static_cast<int32_t>(kForegroundTop), 1.0f, 1, 0, {0, 0}};
    const photoshopapi_c_layer_options group_options{"Artwork Group", 0, 0, 1.0f, 1, 0, {0, 0}};

    try
    {
        check(photoshopapi_c_document_create(kWidth, kHeight, &document), "create document");
        check(photoshopapi_c_document_set_compression(document, PHOTOSHOPAPI_C_COMPRESSION_RLE), "set RLE compression");

        const photoshopapi_c_rgb8_view background_view{images.background.data(), kWidth, kHeight, kWidth * 3u};
        check(photoshopapi_c_image_layer_create_rgb8(&background_view, &background_options, &background), "create background");
        check(photoshopapi_c_document_add_layer(document, background), "add background");

        const photoshopapi_c_rgba8_view foreground_view{images.foreground.data(), kForegroundWidth, kForegroundHeight, kForegroundWidth * 4u};
        check(photoshopapi_c_image_layer_create_rgba8(&foreground_view, &foreground_options, &foreground), "create foreground");
        if (grouped)
        {
            check(photoshopapi_c_group_layer_create(&group_options, &group), "create group");
            check(photoshopapi_c_document_add_layer(document, group), "add group");
            check(photoshopapi_c_group_add_layer(group, foreground), "add foreground to group");
        }
        else
        {
            check(photoshopapi_c_document_add_layer(document, foreground), "add foreground");
        }

        if (masked)
        {
            const photoshopapi_c_mask8_view mask_view{images.mask.data(), kForegroundWidth, kForegroundHeight, kForegroundWidth};
            check(photoshopapi_c_layer_set_mask8(foreground, &mask_view), "set foreground mask");
        }

        const photoshopapi_c_rgb8_view merged_view{images.merged.data(), kWidth, kHeight, kWidth * 3u};
        check(photoshopapi_c_document_set_merged_rgb8(document, &merged_view), "set merged image");
        const auto path_u8 = path.u8string();
        const std::string utf8_path(reinterpret_cast<const char*>(path_u8.data()), path_u8.size());
        check(photoshopapi_c_document_write(document, utf8_path.c_str()), "write fixture");
    }
    catch (...)
    {
        photoshopapi_c_layer_destroy(foreground);
        photoshopapi_c_layer_destroy(background);
        photoshopapi_c_layer_destroy(group);
        photoshopapi_c_document_destroy(document);
        throw;
    }

    photoshopapi_c_layer_destroy(foreground);
    photoshopapi_c_layer_destroy(background);
    photoshopapi_c_layer_destroy(group);
    photoshopapi_c_document_destroy(document);
}
}

int main(int argc, char** argv)
{
    try
    {
        const auto output = std::filesystem::path(argc > 1 ? argv[1] : "psd-fixtures");
        std::filesystem::create_directories(output);
        write_fixture(output / "psd_compatibility_fixture.psd", false, false);
        write_fixture(output / "psd_feature_fixture.psd", true, true);
        std::ofstream manifest(output / "manifest.json", std::ios::binary);
        manifest << "{\n"
                    "  \"canvas\": {\"width\": 256, \"height\": 192, \"colorMode\": \"RGB\", \"bitDepth\": 8},\n"
                    "  \"compression\": \"RLE\",\n"
                    "  \"fixtures\": [\n"
                    "    {\"file\": \"psd_compatibility_fixture.psd\", \"layers\": [\"Foreground\", \"Background\"], \"purpose\": \"baseline\"},\n"
                    "    {\"file\": \"psd_feature_fixture.psd\", \"layers\": [\"Artwork Group/Foreground\", \"Background\"], \"purpose\": \"group and mask\"}\n"
                    "  ]\n}\n";
        return 0;
    }
    catch (const std::exception& exception)
    {
        return (std::fprintf(stderr, "%s\n", exception.what()), 1);
    }
}
