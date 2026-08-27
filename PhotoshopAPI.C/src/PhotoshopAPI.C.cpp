#include <PhotoshopAPI.C/PhotoshopAPI.C.h>
#include "PhotoshopAPI.C.Internal.h"

#include <cstring>
#include <filesystem>
#include <limits>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{

class invalid_argument_error final : public std::invalid_argument
{
public:
    using std::invalid_argument::invalid_argument;
};

class dimensions_error final : public std::invalid_argument
{
public:
    using std::invalid_argument::invalid_argument;
};

class ownership_error final : public std::invalid_argument
{
public:
    using std::invalid_argument::invalid_argument;
};

const char* layer_name(
    const photoshopapi_c_layer_options* options,
    const char* fallback)
{
    if (options == nullptr || options->name == nullptr || options->name[0] == '\0')
    {
        return fallback;
    }
    return options->name;
}

psapi_layer_type::Params make_params(
    const photoshopapi_c_layer_options* options,
    const char* fallback)
{
    psapi_layer_type::Params params{};
    params.name = layer_name(options, fallback);
    params.colormode = psapi::Enum::ColorMode::RGB;
    params.opacity = 255u;
    params.visible = true;
    params.locked = false;

    if (options != nullptr)
    {
        if (!(options->opacity >= 0.0f && options->opacity <= 1.0f))
        {
            throw invalid_argument_error("layer opacity must be in the range [0, 1]");
        }
        params.opacity = static_cast<uint8_t>(options->opacity * 255.0f);
        params.visible = options->visible != 0;
        params.locked = options->locked != 0;
    }
    return params;
}

void set_layer_position(
    psapi_layer_type& layer,
    const photoshopapi_c_layer_options* options)
{
    if (options == nullptr)
    {
        return;
    }

    const auto width = static_cast<int64_t>(layer.width());
    const auto height = static_cast<int64_t>(layer.height());
    const auto center_x = static_cast<int64_t>(options->left) + width / 2;
    const auto center_y = static_cast<int64_t>(options->top) + height / 2;
    if (center_x < std::numeric_limits<int32_t>::min() ||
        center_x > std::numeric_limits<int32_t>::max() ||
        center_y < std::numeric_limits<int32_t>::min() ||
        center_y > std::numeric_limits<int32_t>::max())
    {
        throw dimensions_error("layer position is outside the supported coordinate range");
    }
    layer.center_x(static_cast<float>(center_x));
    layer.center_y(static_cast<float>(center_y));
}

size_t checked_pixel_count(uint32_t width, uint32_t height)
{
    const auto width_size = static_cast<size_t>(width);
    const auto height_size = static_cast<size_t>(height);
    if (height_size != 0 && width_size > std::numeric_limits<size_t>::max() / height_size)
    {
        throw dimensions_error("pixel dimensions overflow the host size type");
    }
    return width_size * height_size;
}

template <typename View>
uint64_t validate_view(const View& view, uint64_t bytes_per_pixel, const char* name)
{
    if (view.width == 0 || view.height == 0)
    {
        throw dimensions_error(std::string(name) + " dimensions must be non-zero");
    }
    if (view.pixels == nullptr)
    {
        throw invalid_argument_error(std::string(name) + ".pixels must not be null");
    }

    const auto minimum_stride = static_cast<uint64_t>(view.width) * bytes_per_pixel;
    const auto stride = view.stride_bytes == 0 ? minimum_stride : view.stride_bytes;
    if (stride < minimum_stride)
    {
        throw dimensions_error(std::string(name) + ".stride_bytes is smaller than one packed row");
    }
    if (view.height > 1 && stride > std::numeric_limits<uint64_t>::max() / view.height)
    {
        throw dimensions_error(std::string(name) + " stride is too large");
    }
    return stride;
}

std::vector<uint8_t> copy_gray_view(const photoshopapi_c_mask8_view& view)
{
    const auto stride = validate_view(view, 1u, "mask");
    const auto pixel_count = checked_pixel_count(view.width, view.height);
    std::vector<uint8_t> result(pixel_count);
    for (uint32_t y = 0; y < view.height; ++y)
    {
        std::memcpy(
            result.data() + static_cast<size_t>(y) * view.width,
            view.pixels + static_cast<size_t>(y) * stride,
            view.width);
    }
    return result;
}

std::unordered_map<psapi::Enum::ChannelID, std::vector<uint8_t>> copy_rgba_view(
    const photoshopapi_c_rgba8_view& view)
{
    const auto stride = validate_view(view, 4u, "source");
    const auto pixel_count = checked_pixel_count(view.width, view.height);
    std::unordered_map<psapi::Enum::ChannelID, std::vector<uint8_t>> channels;
    channels[psapi::Enum::ChannelID::Red].resize(pixel_count);
    channels[psapi::Enum::ChannelID::Green].resize(pixel_count);
    channels[psapi::Enum::ChannelID::Blue].resize(pixel_count);
    channels[psapi::Enum::ChannelID::Alpha].resize(pixel_count);

    for (uint32_t y = 0; y < view.height; ++y)
    {
        const auto* row = view.pixels + static_cast<size_t>(y) * stride;
        for (uint32_t x = 0; x < view.width; ++x)
        {
            const auto source_index = static_cast<size_t>(x) * 4u;
            const auto target_index = static_cast<size_t>(y) * view.width + x;
            channels[psapi::Enum::ChannelID::Red][target_index] = row[source_index];
            channels[psapi::Enum::ChannelID::Green][target_index] = row[source_index + 1u];
            channels[psapi::Enum::ChannelID::Blue][target_index] = row[source_index + 2u];
            channels[psapi::Enum::ChannelID::Alpha][target_index] = row[source_index + 3u];
        }
    }
    return channels;
}

template <typename Callable>
photoshopapi_c_status invoke(std::string& error, Callable&& callable) noexcept
{
    try
    {
        callable();
        error.clear();
        return PHOTOSHOPAPI_C_STATUS_SUCCESS;
    }
    catch (const dimensions_error& exception)
    {
        error = exception.what();
        return PHOTOSHOPAPI_C_STATUS_DIMENSIONS_INVALID;
    }
    catch (const ownership_error& exception)
    {
        error = exception.what();
        return PHOTOSHOPAPI_C_STATUS_OWNERSHIP_ERROR;
    }
    catch (const invalid_argument_error& exception)
    {
        error = exception.what();
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    catch (const std::bad_alloc& exception)
    {
        error = exception.what();
        return PHOTOSHOPAPI_C_STATUS_ALLOCATION_FAILED;
    }
    catch (const std::filesystem::filesystem_error& exception)
    {
        error = exception.what();
        return PHOTOSHOPAPI_C_STATUS_IO_ERROR;
    }
    catch (const std::exception& exception)
    {
        error = exception.what();
        return PHOTOSHOPAPI_C_STATUS_WRITE_FAILED;
    }
    catch (...)
    {
        error = "unknown exception crossed the PhotoshopAPI C ABI boundary";
        return PHOTOSHOPAPI_C_STATUS_WRITE_FAILED;
    }
}

photoshopapi_c_status copy_error(
    const std::string& error,
    char* buffer,
    uint32_t buffer_capacity,
    uint32_t* out_required_buffer_size)
{
    if (out_required_buffer_size == nullptr)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    if (error.size() > std::numeric_limits<uint32_t>::max() - 1u)
    {
        return PHOTOSHOPAPI_C_STATUS_BUFFER_TOO_SMALL;
    }

    const auto required = static_cast<uint32_t>(error.size() + 1u);
    *out_required_buffer_size = required;
    if (buffer == nullptr && buffer_capacity == 0)
    {
        return PHOTOSHOPAPI_C_STATUS_SUCCESS;
    }
    if (buffer == nullptr || buffer_capacity < required)
    {
        return PHOTOSHOPAPI_C_STATUS_BUFFER_TOO_SMALL;
    }
    std::memcpy(buffer, error.c_str(), required);
    return PHOTOSHOPAPI_C_STATUS_SUCCESS;
}

bool document_is_usable(const photoshopapi_c_document_state& document)
{
    return !document.write_attempted && document.value != nullptr;
}

} // namespace

extern "C"
{

PHOTOSHOPAPI_C_API uint32_t PHOTOSHOPAPI_C_CALL photoshopapi_c_get_abi_version(void)
{
    return PHOTOSHOPAPI_C_ABI_VERSION;
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_create(
    uint32_t width,
    uint32_t height,
    photoshopapi_c_document** out_document)
{
    if (out_document == nullptr)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    *out_document = nullptr;
    if (width == 0 || height == 0 || width > 300000u || height > 300000u)
    {
        return PHOTOSHOPAPI_C_STATUS_DIMENSIONS_INVALID;
    }

    photoshopapi_c_document* result = nullptr;
    std::string error;
    const auto status = invoke(
        error,
        [&]
        {
            auto document = std::make_unique<photoshopapi_c_document>();
            document->state = std::make_shared<photoshopapi_c_document_state>();
            document->state->value = std::make_unique<psapi_document_type>(
                psapi::Enum::ColorMode::RGB,
                width,
                height);
            result = document.release();
        });
    if (status == PHOTOSHOPAPI_C_STATUS_SUCCESS)
    {
        *out_document = result;
    }
    else
    {
        delete result;
    }
    return status;
}

PHOTOSHOPAPI_C_API void PHOTOSHOPAPI_C_CALL photoshopapi_c_document_destroy(
    photoshopapi_c_document* document)
{
    delete document;
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_group_layer_create(
    const photoshopapi_c_layer_options* options,
    photoshopapi_c_layer** out_layer)
{
    if (out_layer == nullptr)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    *out_layer = nullptr;
    photoshopapi_c_layer* result = nullptr;
    std::string error;
    const auto status = invoke(
        error,
        [&]
        {
            auto params = make_params(options, "Group");
            auto group = std::make_shared<psapi_group_type>(params);
            auto layer = std::make_unique<photoshopapi_c_layer>();
            layer->value = std::move(group);
            result = layer.release();
        });
    if (status == PHOTOSHOPAPI_C_STATUS_SUCCESS)
    {
        *out_layer = result;
    }
    else
    {
        delete result;
    }
    return status;
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_image_layer_create_rgba8(
    const photoshopapi_c_rgba8_view* source,
    const photoshopapi_c_layer_options* options,
    photoshopapi_c_layer** out_layer)
{
    if (source == nullptr || out_layer == nullptr)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    *out_layer = nullptr;
    photoshopapi_c_layer* result = nullptr;
    std::string error;
    const auto status = invoke(
        error,
        [&]
        {
            auto params = make_params(options, "Image");
            params.width = source->width;
            params.height = source->height;
            auto channels = copy_rgba_view(*source);
            auto image = std::make_shared<psapi::ImageLayer<uint8_t>>(
                std::move(channels),
                params);
            set_layer_position(*image, options);
            auto layer = std::make_unique<photoshopapi_c_layer>();
            layer->value = std::move(image);
            result = layer.release();
        });
    if (status == PHOTOSHOPAPI_C_STATUS_SUCCESS)
    {
        *out_layer = result;
    }
    else
    {
        delete result;
    }
    return status;
}

PHOTOSHOPAPI_C_API void PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_destroy(
    photoshopapi_c_layer* layer)
{
    delete layer;
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_add_layer(
    photoshopapi_c_document* document,
    photoshopapi_c_layer* layer)
{
    if (document == nullptr || layer == nullptr)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    return invoke(
        document->last_error,
        [&]
        {
            if (document->state == nullptr || !document_is_usable(*document->state))
            {
                throw ownership_error("document has already been consumed by a write attempt");
            }
            if (layer->value == nullptr || layer->attached)
            {
                throw ownership_error("layer is already attached or invalid");
            }
            if (const auto owner = layer->document.lock();
                owner != nullptr && owner != document->state)
            {
                throw ownership_error("layer belongs to another document");
            }
            document->state->value->add_layer(layer->value);
            layer->document = document->state;
            layer->attached = true;
        });
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_group_add_layer(
    photoshopapi_c_layer* group,
    photoshopapi_c_layer* child)
{
    if (group == nullptr || child == nullptr)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    return invoke(
        group->last_error,
        [&]
        {
            if (group->value == nullptr || child->value == nullptr || child->attached)
            {
                throw ownership_error("group or child is invalid, or the child is already attached");
            }
            const auto document = group->document.lock();
            if (document == nullptr || !document_is_usable(*document))
            {
                throw ownership_error("group must be attached to a usable document before adding children");
            }
            if (const auto owner = child->document.lock();
                owner != nullptr && owner != document)
            {
                throw ownership_error("child belongs to another document");
            }
            auto typed_group = std::dynamic_pointer_cast<psapi_group_type>(group->value);
            if (typed_group == nullptr)
            {
                throw invalid_argument_error("parent layer is not a group layer");
            }
            typed_group->add_layer(*document->value, child->value);
            child->document = document;
            child->attached = true;
        });
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_set_mask8(
    photoshopapi_c_layer* layer,
    const photoshopapi_c_mask8_view* mask)
{
    if (layer == nullptr || mask == nullptr)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    return invoke(
        layer->last_error,
        [&]
        {
            if (layer->value == nullptr)
            {
                throw invalid_argument_error("layer is invalid");
            }
            auto data = copy_gray_view(*mask);
            layer->value->set_mask(data, mask->width, mask->height);
            layer->value->mask_position(psapi::Geometry::Point2D<double>(
                static_cast<double>(layer->value->center_x()),
                static_cast<double>(layer->value->center_y())));
        });
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_write(
    photoshopapi_c_document* document,
    const char* utf8_path)
{
    return photoshopapi_c_document_write_ex(document, utf8_path, 1u);
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_get_last_error(
    const photoshopapi_c_document* document,
    char* buffer,
    uint32_t buffer_capacity,
    uint32_t* out_required_buffer_size)
{
    if (document == nullptr)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    return copy_error(
        document->last_error,
        buffer,
        buffer_capacity,
        out_required_buffer_size);
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_get_last_error(
    const photoshopapi_c_layer* layer,
    char* buffer,
    uint32_t buffer_capacity,
    uint32_t* out_required_buffer_size)
{
    if (layer == nullptr)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    return copy_error(
        layer->last_error,
        buffer,
        buffer_capacity,
        out_required_buffer_size);
}

} // extern "C"
