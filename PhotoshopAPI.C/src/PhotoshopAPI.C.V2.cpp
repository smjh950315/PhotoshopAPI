#include <PhotoshopAPI.C/PhotoshopAPI.C.h>
#include "PhotoshopAPI.C.Internal.h"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <limits>
#include <new>
#include <optional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace
{

class type_mismatch_error final : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

class read_error final : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

template <typename Callable>
photoshopapi_c_status invoke_v2(
    std::string& error,
    photoshopapi_c_status fallback,
    Callable&& callable) noexcept
{
    try
    {
        callable();
        error.clear();
        return PHOTOSHOPAPI_C_STATUS_SUCCESS;
    }
    catch (const type_mismatch_error& exception)
    {
        error = exception.what();
        return PHOTOSHOPAPI_C_STATUS_TYPE_MISMATCH;
    }
    catch (const read_error& exception)
    {
        error = exception.what();
        return PHOTOSHOPAPI_C_STATUS_READ_FAILED;
    }
    catch (const std::invalid_argument& exception)
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
        return fallback;
    }
    catch (...)
    {
        error = "unknown exception crossed the PhotoshopAPI C ABI boundary";
        return fallback;
    }
}

template <typename Callable>
decltype(auto) with_document(photoshopapi_c_document_state& state, Callable&& callable)
{
    if (state.write_attempted)
    {
        throw std::runtime_error("document has already been consumed by a write attempt");
    }
    if (state.value)
    {
        return callable(*state.value);
    }
    if (state.value16)
    {
        return callable(*state.value16);
    }
    if (state.value32)
    {
        return callable(*state.value32);
    }
    throw std::runtime_error("document is no longer usable");
}

template <typename Callable>
decltype(auto) with_document(const photoshopapi_c_document_state& state, Callable&& callable)
{
    if (state.write_attempted)
    {
        throw std::runtime_error("document has already been consumed by a write attempt");
    }
    if (state.value)
    {
        return callable(*state.value);
    }
    if (state.value16)
    {
        return callable(*state.value16);
    }
    if (state.value32)
    {
        return callable(*state.value32);
    }
    throw std::runtime_error("document is no longer usable");
}

template <typename Callable>
decltype(auto) with_layer(photoshopapi_c_layer& layer, Callable&& callable)
{
    if (layer.value)
    {
        return callable(layer.value);
    }
    if (layer.value16)
    {
        return callable(layer.value16);
    }
    if (layer.value32)
    {
        return callable(layer.value32);
    }
    throw std::runtime_error("layer is no longer usable");
}

template <typename Callable>
decltype(auto) with_layer(const photoshopapi_c_layer& layer, Callable&& callable)
{
    if (layer.value)
    {
        return callable(layer.value);
    }
    if (layer.value16)
    {
        return callable(layer.value16);
    }
    if (layer.value32)
    {
        return callable(layer.value32);
    }
    throw std::runtime_error("layer is no longer usable");
}

template <typename Callable>
decltype(auto) with_document_layer(
    photoshopapi_c_document_state& document,
    photoshopapi_c_layer& layer,
    Callable&& callable)
{
    if (document.value && layer.value)
    {
        return callable(*document.value, layer.value);
    }
    if (document.value16 && layer.value16)
    {
        return callable(*document.value16, layer.value16);
    }
    if (document.value32 && layer.value32)
    {
        return callable(*document.value32, layer.value32);
    }
    throw type_mismatch_error("document and layer bit depths do not match");
}

template <typename T>
constexpr photoshopapi_c_bit_depth bit_depth_for()
{
    if constexpr (std::is_same_v<T, psapi::bpp8_t>)
    {
        return PHOTOSHOPAPI_C_BIT_DEPTH_8;
    }
    else if constexpr (std::is_same_v<T, psapi::bpp16_t>)
    {
        return PHOTOSHOPAPI_C_BIT_DEPTH_16;
    }
    else
    {
        return PHOTOSHOPAPI_C_BIT_DEPTH_32;
    }
}

photoshopapi_c_color_mode to_c_color_mode(psapi::Enum::ColorMode mode)
{
    switch (mode)
    {
    case psapi::Enum::ColorMode::Bitmap: return PHOTOSHOPAPI_C_COLOR_MODE_BITMAP;
    case psapi::Enum::ColorMode::Grayscale: return PHOTOSHOPAPI_C_COLOR_MODE_GRAYSCALE;
    case psapi::Enum::ColorMode::Indexed: return PHOTOSHOPAPI_C_COLOR_MODE_INDEXED;
    case psapi::Enum::ColorMode::RGB: return PHOTOSHOPAPI_C_COLOR_MODE_RGB;
    case psapi::Enum::ColorMode::CMYK: return PHOTOSHOPAPI_C_COLOR_MODE_CMYK;
    case psapi::Enum::ColorMode::Multichannel: return PHOTOSHOPAPI_C_COLOR_MODE_MULTICHANNEL;
    case psapi::Enum::ColorMode::Duotone: return PHOTOSHOPAPI_C_COLOR_MODE_DUOTONE;
    case psapi::Enum::ColorMode::Lab: return PHOTOSHOPAPI_C_COLOR_MODE_LAB;
    default: throw std::invalid_argument("unsupported Photoshop color mode");
    }
}

psapi::Enum::ColorMode from_c_color_mode(photoshopapi_c_color_mode mode)
{
    switch (mode)
    {
    case PHOTOSHOPAPI_C_COLOR_MODE_BITMAP: return psapi::Enum::ColorMode::Bitmap;
    case PHOTOSHOPAPI_C_COLOR_MODE_GRAYSCALE: return psapi::Enum::ColorMode::Grayscale;
    case PHOTOSHOPAPI_C_COLOR_MODE_INDEXED: return psapi::Enum::ColorMode::Indexed;
    case PHOTOSHOPAPI_C_COLOR_MODE_RGB: return psapi::Enum::ColorMode::RGB;
    case PHOTOSHOPAPI_C_COLOR_MODE_CMYK: return psapi::Enum::ColorMode::CMYK;
    case PHOTOSHOPAPI_C_COLOR_MODE_MULTICHANNEL: return psapi::Enum::ColorMode::Multichannel;
    case PHOTOSHOPAPI_C_COLOR_MODE_DUOTONE: return psapi::Enum::ColorMode::Duotone;
    case PHOTOSHOPAPI_C_COLOR_MODE_LAB: return psapi::Enum::ColorMode::Lab;
    default: throw std::invalid_argument("invalid Photoshop color mode");
    }
}

psapi::Enum::Compression from_c_compression(photoshopapi_c_compression compression)
{
    switch (compression)
    {
    case PHOTOSHOPAPI_C_COMPRESSION_RAW: return psapi::Enum::Compression::Raw;
    case PHOTOSHOPAPI_C_COMPRESSION_RLE: return psapi::Enum::Compression::Rle;
    case PHOTOSHOPAPI_C_COMPRESSION_ZIP: return psapi::Enum::Compression::Zip;
    case PHOTOSHOPAPI_C_COMPRESSION_ZIP_PREDICTION: return psapi::Enum::Compression::ZipPrediction;
    default: throw std::invalid_argument("invalid compression value");
    }
}

template <typename T>
photoshopapi_c_layer_type layer_type(const std::shared_ptr<psapi::Layer<T>>& layer)
{
    if (std::dynamic_pointer_cast<psapi::TextLayer<T>>(layer)) return PHOTOSHOPAPI_C_LAYER_TYPE_TEXT;
    if (std::dynamic_pointer_cast<psapi::SmartObjectLayer<T>>(layer)) return PHOTOSHOPAPI_C_LAYER_TYPE_SMART_OBJECT;
    if (std::dynamic_pointer_cast<psapi::ArtboardLayer<T>>(layer)) return PHOTOSHOPAPI_C_LAYER_TYPE_ARTBOARD;
    if (std::dynamic_pointer_cast<psapi::GroupLayer<T>>(layer)) return PHOTOSHOPAPI_C_LAYER_TYPE_GROUP;
    if (std::dynamic_pointer_cast<psapi::ImageLayer<T>>(layer)) return PHOTOSHOPAPI_C_LAYER_TYPE_IMAGE;
    if (std::dynamic_pointer_cast<psapi::ShapeLayer<T>>(layer)) return PHOTOSHOPAPI_C_LAYER_TYPE_SHAPE;
    if (std::dynamic_pointer_cast<psapi::AdjustmentLayer<T>>(layer)) return PHOTOSHOPAPI_C_LAYER_TYPE_ADJUSTMENT;
    if (std::dynamic_pointer_cast<psapi::SectionDividerLayer<T>>(layer)) return PHOTOSHOPAPI_C_LAYER_TYPE_SECTION_DIVIDER;
    return PHOTOSHOPAPI_C_LAYER_TYPE_UNKNOWN;
}

template <typename T>
photoshopapi_c_layer* make_layer_handle(
    std::shared_ptr<psapi::Layer<T>> value,
    const std::shared_ptr<photoshopapi_c_document_state>& document,
    bool attached)
{
    auto result = std::make_unique<photoshopapi_c_layer>();
    if constexpr (std::is_same_v<T, psapi::bpp8_t>)
    {
        result->value = std::move(value);
    }
    else if constexpr (std::is_same_v<T, psapi::bpp16_t>)
    {
        result->value16 = std::move(value);
    }
    else
    {
        result->value32 = std::move(value);
    }
    result->document = document;
    result->attached = attached;
    return result.release();
}

photoshopapi_c_status copy_string(
    const std::string& value,
    char* buffer,
    uint32_t buffer_capacity,
    uint32_t* out_required_size)
{
    if (out_required_size == nullptr || value.size() > std::numeric_limits<uint32_t>::max() - 1u)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    const auto required = static_cast<uint32_t>(value.size() + 1u);
    *out_required_size = required;
    if (buffer == nullptr && buffer_capacity == 0)
    {
        return PHOTOSHOPAPI_C_STATUS_SUCCESS;
    }
    if (buffer == nullptr || buffer_capacity < required)
    {
        return PHOTOSHOPAPI_C_STATUS_BUFFER_TOO_SMALL;
    }
    std::memcpy(buffer, value.c_str(), required);
    return PHOTOSHOPAPI_C_STATUS_SUCCESS;
}

photoshopapi_c_status copy_bytes(
    const void* data,
    uint64_t size,
    void* buffer,
    uint64_t buffer_capacity,
    uint64_t* out_required_size)
{
    if (out_required_size == nullptr)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    *out_required_size = size;
    if (buffer == nullptr && buffer_capacity == 0)
    {
        return PHOTOSHOPAPI_C_STATUS_SUCCESS;
    }
    if (buffer == nullptr || buffer_capacity < size)
    {
        return PHOTOSHOPAPI_C_STATUS_BUFFER_TOO_SMALL;
    }
    if (size != 0)
    {
        std::memcpy(buffer, data, static_cast<size_t>(size));
    }
    return PHOTOSHOPAPI_C_STATUS_SUCCESS;
}

std::string path_to_utf8(const std::filesystem::path& path)
{
    const auto encoded = path.u8string();
    return std::string(reinterpret_cast<const char*>(encoded.data()), encoded.size());
}

template <typename Document>
constexpr photoshopapi_c_bit_depth document_bit_depth_for()
{
    if constexpr (std::is_same_v<std::remove_cvref_t<Document>, psapi_document_type>)
    {
        return PHOTOSHOPAPI_C_BIT_DEPTH_8;
    }
    else if constexpr (std::is_same_v<std::remove_cvref_t<Document>, psapi_document16_type>)
    {
        return PHOTOSHOPAPI_C_BIT_DEPTH_16;
    }
    else
    {
        return PHOTOSHOPAPI_C_BIT_DEPTH_32;
    }
}

template <typename Document>
struct document_sample;

template <>
struct document_sample<psapi_document_type> { using type = psapi::bpp8_t; };
template <>
struct document_sample<psapi_document16_type> { using type = psapi::bpp16_t; };
template <>
struct document_sample<psapi_document32_type> { using type = psapi::bpp32_t; };

template <typename Document>
using document_sample_t = typename document_sample<std::remove_cvref_t<Document>>::type;

template <typename T>
std::shared_ptr<psapi::GroupLayer<T>> require_group(const std::shared_ptr<psapi::Layer<T>>& layer)
{
    auto group = std::dynamic_pointer_cast<psapi::GroupLayer<T>>(layer);
    if (!group)
    {
        throw type_mismatch_error("layer is not a group layer");
    }
    return group;
}

template <typename T>
psapi::ImageDataMixin<T>& require_image_data(const std::shared_ptr<psapi::Layer<T>>& layer)
{
    auto* image = dynamic_cast<psapi::ImageDataMixin<T>*>(layer.get());
    if (!image)
    {
        throw type_mismatch_error("layer does not expose image channel data");
    }
    return *image;
}

template <typename T>
psapi::WritableImageDataMixin<T>& require_writable_image_data(const std::shared_ptr<psapi::Layer<T>>& layer)
{
    auto* image = dynamic_cast<psapi::WritableImageDataMixin<T>*>(layer.get());
    if (!image)
    {
        throw type_mismatch_error("layer image channel data is read-only");
    }
    return *image;
}

template <typename T>
std::shared_ptr<psapi::TextLayer<T>> require_text_layer(const std::shared_ptr<psapi::Layer<T>>& layer)
{
    auto text = std::dynamic_pointer_cast<psapi::TextLayer<T>>(layer);
    if (!text)
    {
        throw type_mismatch_error("layer is not a text layer");
    }
    return text;
}

template <typename T>
std::shared_ptr<psapi::SmartObjectLayer<T>> require_smart_object(const std::shared_ptr<psapi::Layer<T>>& layer)
{
    auto smart_object = std::dynamic_pointer_cast<psapi::SmartObjectLayer<T>>(layer);
    if (!smart_object)
    {
        throw type_mismatch_error("layer is not a smart object layer");
    }
    return smart_object;
}

} // namespace

extern "C"
{

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_create_ex(
    photoshopapi_c_bit_depth bit_depth,
    photoshopapi_c_color_mode color_mode,
    uint64_t width,
    uint64_t height,
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

    std::string error;
    photoshopapi_c_document* result = nullptr;
    const auto status = invoke_v2(error, PHOTOSHOPAPI_C_STATUS_NOT_SUPPORTED, [&]
    {
        auto document = std::make_unique<photoshopapi_c_document>();
        document->state = std::make_shared<photoshopapi_c_document_state>();
        const auto native_color_mode = from_c_color_mode(color_mode);
        switch (bit_depth)
        {
        case PHOTOSHOPAPI_C_BIT_DEPTH_8:
            document->state->value = std::make_unique<psapi_document_type>(native_color_mode, width, height);
            break;
        case PHOTOSHOPAPI_C_BIT_DEPTH_16:
            document->state->value16 = std::make_unique<psapi_document16_type>(native_color_mode, width, height);
            break;
        case PHOTOSHOPAPI_C_BIT_DEPTH_32:
            document->state->value32 = std::make_unique<psapi_document32_type>(native_color_mode, width, height);
            break;
        default:
            throw std::invalid_argument("bit depth must be 8, 16, or 32");
        }
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

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_read(
    const char* utf8_path,
    photoshopapi_c_document** out_document)
{
    if (utf8_path == nullptr || utf8_path[0] == '\0' || out_document == nullptr)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    *out_document = nullptr;
    std::string error;
    photoshopapi_c_document* result = nullptr;
    const auto status = invoke_v2(error, PHOTOSHOPAPI_C_STATUS_READ_FAILED, [&]
    {
        try
        {
            const auto path = std::filesystem::u8path(utf8_path);
            psapi::ProgressCallback callback{};
            auto input = psapi::File(path);
            auto parsed = std::make_unique<psapi::PhotoshopFile>();
            parsed->read(input, callback);

            auto document = std::make_unique<photoshopapi_c_document>();
            document->state = std::make_shared<photoshopapi_c_document_state>();
            switch (parsed->m_Header.m_Depth)
            {
            case psapi::Enum::BitDepth::BD_8:
                document->state->value = std::make_unique<psapi_document_type>(std::move(parsed), path);
                break;
            case psapi::Enum::BitDepth::BD_16:
                document->state->value16 = std::make_unique<psapi_document16_type>(std::move(parsed), path);
                break;
            case psapi::Enum::BitDepth::BD_32:
                document->state->value32 = std::make_unique<psapi_document32_type>(std::move(parsed), path);
                break;
            default:
                throw read_error("only 8-bit, 16-bit, and 32-bit Photoshop documents are supported");
            }
            result = document.release();
        }
        catch (const read_error&)
        {
            throw;
        }
        catch (const std::exception& exception)
        {
            throw read_error(exception.what());
        }
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

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_get_info(
    const photoshopapi_c_document* document,
    photoshopapi_c_document_info* out_info)
{
    if (!document || !document->state || !out_info)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    auto& error = const_cast<photoshopapi_c_document*>(document)->last_error;
    return invoke_v2(error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        with_document(*document->state, [&](auto& native)
        {
            out_info->width = native.width();
            out_info->height = native.height();
            out_info->dpi = native.dpi();
            out_info->bit_depth = document_bit_depth_for<decltype(native)>();
            out_info->color_mode = to_c_color_mode(native.colormode());
            out_info->root_layer_count = static_cast<uint32_t>(native.layers().size());
        });
    });
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_set_size(
    photoshopapi_c_document* document,
    uint64_t width,
    uint64_t height)
{
    if (!document || !document->state || width == 0 || height == 0 || width > 300000u || height > 300000u)
    {
        return PHOTOSHOPAPI_C_STATUS_DIMENSIONS_INVALID;
    }
    return invoke_v2(document->last_error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        with_document(*document->state, [&](auto& native)
        {
            native.width(width);
            native.height(height);
        });
    });
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_set_dpi(
    photoshopapi_c_document* document,
    float dpi)
{
    if (!document || !document->state || dpi <= 0.0f)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    return invoke_v2(document->last_error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        with_document(*document->state, [&](auto& native) { native.dpi(dpi); });
    });
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_set_compression(
    photoshopapi_c_document* document,
    photoshopapi_c_compression compression)
{
    if (!document || !document->state)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    return invoke_v2(document->last_error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        const auto native_compression = from_c_compression(compression);
        with_document(*document->state, [&](auto& native) { native.set_compression(native_compression); });
    });
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_invalidate_text_cache(
    photoshopapi_c_document* document)
{
    if (!document || !document->state)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    return invoke_v2(document->last_error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        with_document(*document->state, [](auto& native) { native.invalidate_text_cache(); });
    });
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_get_icc_profile(
    const photoshopapi_c_document* document,
    uint8_t* buffer,
    uint64_t buffer_capacity,
    uint64_t* out_required_size)
{
    if (!document || !document->state || !out_required_size)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    photoshopapi_c_status copy_status = PHOTOSHOPAPI_C_STATUS_SUCCESS;
    auto& error = const_cast<photoshopapi_c_document*>(document)->last_error;
    const auto status = invoke_v2(error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        with_document(*document->state, [&](auto& native)
        {
            const auto data = native.icc_profile().data();
            copy_status = copy_bytes(data.data(), data.size(), buffer, buffer_capacity, out_required_size);
        });
    });
    return status == PHOTOSHOPAPI_C_STATUS_SUCCESS ? copy_status : status;
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_set_icc_profile(
    photoshopapi_c_document* document,
    const uint8_t* data,
    uint64_t data_size)
{
    if (!document || !document->state || (data_size != 0 && !data) || data_size > std::numeric_limits<size_t>::max())
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    return invoke_v2(document->last_error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        std::vector<uint8_t> profile;
        if (data_size != 0)
        {
            profile.assign(data, data + static_cast<size_t>(data_size));
        }
        with_document(*document->state, [&](auto& native) { native.icc_profile(psapi::ICCProfile(std::move(profile))); });
    });
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_get_root_layer(
    photoshopapi_c_document* document,
    uint32_t index,
    photoshopapi_c_layer** out_layer)
{
    if (!document || !document->state || !out_layer)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    *out_layer = nullptr;
    return invoke_v2(document->last_error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        with_document(*document->state, [&](auto& native)
        {
            if (index >= native.layers().size())
            {
                throw std::invalid_argument("root layer index is out of range");
            }
            using sample_type = document_sample_t<decltype(native)>;
            *out_layer = make_layer_handle<sample_type>(native.layers()[index], document->state, true);
        });
    });
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_find_layer(
    photoshopapi_c_document* document,
    const char* utf8_path,
    photoshopapi_c_layer** out_layer)
{
    if (!document || !document->state || !utf8_path || utf8_path[0] == '\0' || !out_layer)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    *out_layer = nullptr;
    return invoke_v2(document->last_error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        with_document(*document->state, [&](auto& native)
        {
            auto found = native.find_layer(utf8_path);
            if (!found)
            {
                throw std::invalid_argument("layer path was not found");
            }
            using sample_type = document_sample_t<decltype(native)>;
            *out_layer = make_layer_handle<sample_type>(std::move(found), document->state, true);
        });
    });
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_get_info(
    const photoshopapi_c_layer* layer,
    photoshopapi_c_layer_info* out_info)
{
    if (!layer || !out_info)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    auto& error = const_cast<photoshopapi_c_layer*>(layer)->last_error;
    return invoke_v2(error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        with_layer(*layer, [&](const auto& native)
        {
            using sample_type = typename std::remove_reference_t<decltype(native)>::element_type::value_type;
            out_info->type = layer_type<sample_type>(native);
            out_info->bit_depth = bit_depth_for<sample_type>();
            out_info->color_mode = to_c_color_mode(native->color_mode());
            out_info->blend_mode = static_cast<photoshopapi_c_blend_mode>(native->blendmode());
            out_info->display_color = static_cast<photoshopapi_c_layer_color>(native->display_color());
            out_info->width = native->width();
            out_info->height = native->height();
            out_info->center_x = native->center_x();
            out_info->center_y = native->center_y();
            out_info->opacity = native->opacity();
            out_info->fill = native->fill();
            out_info->visible = native->visible() ? 1u : 0u;
            out_info->locked = native->locked() ? 1u : 0u;
            out_info->clipping_mask = native->clipping_mask() ? 1u : 0u;
            out_info->reserved = 0;
        });
    });
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_set_info(
    photoshopapi_c_layer* layer,
    const photoshopapi_c_layer_info* info)
{
    if (!layer || !info || info->width > 300000u || info->height > 300000u)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    return invoke_v2(layer->last_error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        with_layer(*layer, [&](auto& native)
        {
            native->blendmode(static_cast<psapi::Enum::BlendMode>(info->blend_mode));
            native->display_color(static_cast<psapi::Enum::LayerColor>(info->display_color));
            native->width(info->width);
            native->height(info->height);
            native->center_x(info->center_x);
            native->center_y(info->center_y);
            native->opacity(info->opacity);
            native->fill(info->fill);
            native->visible(info->visible != 0);
            native->locked(info->locked != 0);
            native->clipping_mask(info->clipping_mask != 0);
        });
    });
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_get_name(
    const photoshopapi_c_layer* layer,
    char* buffer,
    uint32_t buffer_capacity,
    uint32_t* out_required_buffer_size)
{
    if (!layer || !out_required_buffer_size)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    photoshopapi_c_status copy_status = PHOTOSHOPAPI_C_STATUS_SUCCESS;
    auto& error = const_cast<photoshopapi_c_layer*>(layer)->last_error;
    const auto status = invoke_v2(error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        with_layer(*layer, [&](const auto& native)
        {
            copy_status = copy_string(native->name(), buffer, buffer_capacity, out_required_buffer_size);
        });
    });
    return status == PHOTOSHOPAPI_C_STATUS_SUCCESS ? copy_status : status;
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_set_name(
    photoshopapi_c_layer* layer,
    const char* utf8_name)
{
    if (!layer || !utf8_name)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    return invoke_v2(layer->last_error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        with_layer(*layer, [&](auto& native) { native->name(utf8_name); });
    });
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_get_child_count(
    const photoshopapi_c_layer* layer,
    uint32_t* out_count)
{
    if (!layer || !out_count)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    auto& error = const_cast<photoshopapi_c_layer*>(layer)->last_error;
    return invoke_v2(error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        with_layer(*layer, [&](const auto& native)
        {
            using sample_type = typename std::remove_reference_t<decltype(native)>::element_type::value_type;
            *out_count = static_cast<uint32_t>(require_group<sample_type>(native)->layers().size());
        });
    });
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_get_child(
    photoshopapi_c_layer* layer,
    uint32_t index,
    photoshopapi_c_layer** out_child)
{
    if (!layer || !out_child)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    *out_child = nullptr;
    return invoke_v2(layer->last_error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        with_layer(*layer, [&](auto& native)
        {
            using sample_type = typename std::remove_reference_t<decltype(native)>::element_type::value_type;
            auto group = require_group<sample_type>(native);
            if (index >= group->layers().size())
            {
                throw std::invalid_argument("child layer index is out of range");
            }
            *out_child = make_layer_handle<sample_type>(group->layers()[index], layer->document.lock(), true);
        });
    });
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_group_get_collapsed(
    const photoshopapi_c_layer* layer,
    uint8_t* out_collapsed)
{
    if (!layer || !out_collapsed)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    auto& error = const_cast<photoshopapi_c_layer*>(layer)->last_error;
    return invoke_v2(error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        with_layer(*layer, [&](const auto& native)
        {
            using sample_type = typename std::remove_reference_t<decltype(native)>::element_type::value_type;
            *out_collapsed = require_group<sample_type>(native)->collapsed() ? 1u : 0u;
        });
    });
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_group_set_collapsed(
    photoshopapi_c_layer* layer,
    uint8_t collapsed)
{
    if (!layer)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    return invoke_v2(layer->last_error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        with_layer(*layer, [&](auto& native)
        {
            using sample_type = typename std::remove_reference_t<decltype(native)>::element_type::value_type;
            require_group<sample_type>(native)->collapsed(collapsed != 0);
        });
    });
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_remove_layer(
    photoshopapi_c_document* document,
    photoshopapi_c_layer* layer)
{
    if (!document || !document->state || !layer)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    return invoke_v2(document->last_error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        with_document_layer(*document->state, *layer, [](auto& native_document, auto& native_layer)
        {
            native_document.remove_layer(native_layer);
        });
        layer->attached = false;
    });
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_move_layer(
    photoshopapi_c_document* document,
    photoshopapi_c_layer* layer,
    photoshopapi_c_layer* new_parent)
{
    if (!document || !document->state || !layer)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    return invoke_v2(document->last_error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        if (document->state->value && layer->value)
        {
            if (new_parent && !new_parent->value) throw type_mismatch_error("parent layer bit depth does not match document");
            document->state->value->move_layer(layer->value, new_parent ? new_parent->value : nullptr);
        }
        else if (document->state->value16 && layer->value16)
        {
            if (new_parent && !new_parent->value16) throw type_mismatch_error("parent layer bit depth does not match document");
            document->state->value16->move_layer(layer->value16, new_parent ? new_parent->value16 : nullptr);
        }
        else if (document->state->value32 && layer->value32)
        {
            if (new_parent && !new_parent->value32) throw type_mismatch_error("parent layer bit depth does not match document");
            document->state->value32->move_layer(layer->value32, new_parent ? new_parent->value32 : nullptr);
        }
        else
        {
            throw type_mismatch_error("document and layer bit depths do not match");
        }
        layer->attached = true;
    });
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_get_channel_indices(
    photoshopapi_c_layer* layer,
    int32_t* buffer,
    uint32_t buffer_capacity,
    uint32_t* out_required_count)
{
    if (!layer || !out_required_count)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    photoshopapi_c_status copy_status = PHOTOSHOPAPI_C_STATUS_SUCCESS;
    const auto status = invoke_v2(layer->last_error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        with_layer(*layer, [&](auto& native)
        {
            using sample_type = typename std::remove_reference_t<decltype(native)>::element_type::value_type;
            auto indices = require_image_data<sample_type>(native).channel_indices(true);
            if (indices.size() > std::numeric_limits<uint32_t>::max()) throw std::runtime_error("too many channels");
            *out_required_count = static_cast<uint32_t>(indices.size());
            if (!buffer && buffer_capacity == 0) return;
            if (!buffer || buffer_capacity < indices.size())
            {
                copy_status = PHOTOSHOPAPI_C_STATUS_BUFFER_TOO_SMALL;
                return;
            }
            std::copy(indices.begin(), indices.end(), buffer);
        });
    });
    return status == PHOTOSHOPAPI_C_STATUS_SUCCESS ? copy_status : status;
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_get_channel_data(
    photoshopapi_c_layer* layer,
    int32_t channel_index,
    void* buffer,
    uint64_t buffer_capacity,
    uint64_t* out_required_size)
{
    if (!layer || !out_required_size)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    photoshopapi_c_status copy_status = PHOTOSHOPAPI_C_STATUS_SUCCESS;
    const auto status = invoke_v2(layer->last_error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        with_layer(*layer, [&](auto& native)
        {
            using sample_type = typename std::remove_reference_t<decltype(native)>::element_type::value_type;
            auto data = require_image_data<sample_type>(native).get_channel(channel_index);
            copy_status = copy_bytes(data.data(), data.size() * sizeof(sample_type), buffer, buffer_capacity, out_required_size);
        });
    });
    return status == PHOTOSHOPAPI_C_STATUS_SUCCESS ? copy_status : status;
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_set_channel_data(
    photoshopapi_c_layer* layer,
    int32_t channel_index,
    const void* data,
    uint64_t data_size)
{
    if (!layer || (data_size != 0 && !data))
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    return invoke_v2(layer->last_error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        with_layer(*layer, [&](auto& native)
        {
            using sample_type = typename std::remove_reference_t<decltype(native)>::element_type::value_type;
            if (data_size % sizeof(sample_type) != 0) throw std::invalid_argument("channel byte size is not aligned to the document sample type");
            const auto count = static_cast<size_t>(data_size / sizeof(sample_type));
            auto samples = std::span<const sample_type>(static_cast<const sample_type*>(data), count);
            require_writable_image_data<sample_type>(native).set_channel(channel_index, samples);
        });
    });
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_get_mask_info(
    const photoshopapi_c_layer* layer,
    photoshopapi_c_mask_info* out_info)
{
    if (!layer || !out_info)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    auto& error = const_cast<photoshopapi_c_layer*>(layer)->last_error;
    return invoke_v2(error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        with_layer(*layer, [&](const auto& native)
        {
            out_info->has_mask = native->has_mask() ? 1u : 0u;
            out_info->width = native->has_mask() ? native->mask_width() : 0u;
            out_info->height = native->has_mask() ? native->mask_height() : 0u;
            const auto position = native->has_mask() ? native->mask_position() : psapi::Geometry::Point2D<double>{0.0, 0.0};
            out_info->center_x = position.x;
            out_info->center_y = position.y;
            out_info->disabled = native->mask_disabled() ? 1u : 0u;
            out_info->relative_to_layer = native->mask_relative_to_layer() ? 1u : 0u;
            out_info->default_color = native->mask_default_color();
            const auto density = native->mask_density();
            out_info->has_density = density.has_value() ? 1u : 0u;
            out_info->density = density.value_or(0u);
            const auto feather = native->mask_feather();
            out_info->has_feather = feather.has_value() ? 1u : 0u;
            out_info->feather = feather.value_or(0.0);
            out_info->reserved = 0;
        });
    });
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_set_mask_info(
    photoshopapi_c_layer* layer,
    const photoshopapi_c_mask_info* info)
{
    if (!layer || !info)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    return invoke_v2(layer->last_error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        with_layer(*layer, [&](auto& native)
        {
            if (native->has_mask()) native->mask_position({info->center_x, info->center_y});
            native->mask_disabled(info->disabled != 0);
            native->mask_relative_to_layer(info->relative_to_layer != 0);
            native->mask_default_color(info->default_color);
            native->mask_density(info->has_density ? std::optional<uint8_t>(info->density) : std::nullopt);
            native->mask_feather(info->has_feather ? std::optional<float64_t>(info->feather) : std::nullopt);
        });
    });
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_get_mask_data(
    photoshopapi_c_layer* layer,
    void* buffer,
    uint64_t buffer_capacity,
    uint64_t* out_required_size)
{
    if (!layer || !out_required_size)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    photoshopapi_c_status copy_status = PHOTOSHOPAPI_C_STATUS_SUCCESS;
    const auto status = invoke_v2(layer->last_error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        with_layer(*layer, [&](auto& native)
        {
            using sample_type = typename std::remove_reference_t<decltype(native)>::element_type::value_type;
            auto mask = native->get_mask();
            copy_status = copy_bytes(mask.data(), mask.size() * sizeof(sample_type), buffer, buffer_capacity, out_required_size);
        });
    });
    return status == PHOTOSHOPAPI_C_STATUS_SUCCESS ? copy_status : status;
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_set_mask_data(
    photoshopapi_c_layer* layer,
    const void* data,
    uint64_t data_size,
    uint64_t width,
    uint64_t height)
{
    if (!layer || !data || width == 0 || height == 0 || width > 300000u || height > 300000u)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    return invoke_v2(layer->last_error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        with_layer(*layer, [&](auto& native)
        {
            using sample_type = typename std::remove_reference_t<decltype(native)>::element_type::value_type;
            if (data_size % sizeof(sample_type) != 0 || data_size / sizeof(sample_type) != width * height)
            {
                throw std::invalid_argument("mask byte size does not match width, height, and document sample type");
            }
            const auto samples = std::span<const sample_type>(static_cast<const sample_type*>(data), static_cast<size_t>(width * height));
            native->set_mask(samples, static_cast<size_t>(width), static_cast<size_t>(height));
        });
    });
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_text_layer_get_text(
    const photoshopapi_c_layer* layer,
    char* buffer,
    uint32_t buffer_capacity,
    uint32_t* out_required_buffer_size)
{
    if (!layer || !out_required_buffer_size)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    photoshopapi_c_status copy_status = PHOTOSHOPAPI_C_STATUS_SUCCESS;
    auto& error = const_cast<photoshopapi_c_layer*>(layer)->last_error;
    const auto status = invoke_v2(error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        with_layer(*layer, [&](const auto& native)
        {
            using sample_type = typename std::remove_reference_t<decltype(native)>::element_type::value_type;
            copy_status = copy_string(require_text_layer<sample_type>(native)->text().value_or(""), buffer, buffer_capacity, out_required_buffer_size);
        });
    });
    return status == PHOTOSHOPAPI_C_STATUS_SUCCESS ? copy_status : status;
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_text_layer_set_text(
    photoshopapi_c_layer* layer,
    const char* utf8_text)
{
    if (!layer || !utf8_text)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    return invoke_v2(layer->last_error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        with_layer(*layer, [&](auto& native)
        {
            using sample_type = typename std::remove_reference_t<decltype(native)>::element_type::value_type;
            require_text_layer<sample_type>(native)->set_text(utf8_text);
        });
    });
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_text_layer_replace_text(
    photoshopapi_c_layer* layer,
    const char* utf8_old_text,
    const char* utf8_new_text,
    uint8_t replace_all)
{
    if (!layer || !utf8_old_text || !utf8_new_text)
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    return invoke_v2(layer->last_error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        with_layer(*layer, [&](auto& native)
        {
            using sample_type = typename std::remove_reference_t<decltype(native)>::element_type::value_type;
            require_text_layer<sample_type>(native)->replace_text(utf8_old_text, utf8_new_text, replace_all != 0);
        });
    });
}

#define PHOTOSHOPAPI_C_SMART_STRING_FUNCTION(function_name, expression) \
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL function_name( \
    const photoshopapi_c_layer* layer, char* buffer, uint32_t buffer_capacity, uint32_t* out_required_buffer_size) \
{ \
    if (!layer || !out_required_buffer_size) return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT; \
    photoshopapi_c_status copy_status = PHOTOSHOPAPI_C_STATUS_SUCCESS; \
    auto& error = const_cast<photoshopapi_c_layer*>(layer)->last_error; \
    const auto status = invoke_v2(error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&] \
    { \
        with_layer(*layer, [&](const auto& native) \
        { \
            using sample_type = typename std::remove_reference_t<decltype(native)>::element_type::value_type; \
            auto smart_object = require_smart_object<sample_type>(native); \
            copy_status = copy_string((expression), buffer, buffer_capacity, out_required_buffer_size); \
        }); \
    }); \
    return status == PHOTOSHOPAPI_C_STATUS_SUCCESS ? copy_status : status; \
}

PHOTOSHOPAPI_C_SMART_STRING_FUNCTION(photoshopapi_c_smart_object_get_hash, smart_object->hash())
PHOTOSHOPAPI_C_SMART_STRING_FUNCTION(photoshopapi_c_smart_object_get_filename, smart_object->filename())
PHOTOSHOPAPI_C_SMART_STRING_FUNCTION(photoshopapi_c_smart_object_get_filepath, path_to_utf8(smart_object->filepath()))

#undef PHOTOSHOPAPI_C_SMART_STRING_FUNCTION

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_smart_object_replace(
    photoshopapi_c_layer* layer,
    const char* utf8_path,
    uint8_t link_externally)
{
    if (!layer || !utf8_path || utf8_path[0] == '\0') return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    return invoke_v2(layer->last_error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        with_layer(*layer, [&](auto& native)
        {
            using sample_type = typename std::remove_reference_t<decltype(native)>::element_type::value_type;
            require_smart_object<sample_type>(native)->replace(std::filesystem::u8path(utf8_path), link_externally != 0);
        });
    });
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_smart_object_move(
    photoshopapi_c_layer* layer, double x_offset, double y_offset)
{
    if (!layer) return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    return invoke_v2(layer->last_error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        with_layer(*layer, [&](auto& native)
        {
            using sample_type = typename std::remove_reference_t<decltype(native)>::element_type::value_type;
            require_smart_object<sample_type>(native)->move({x_offset, y_offset});
        });
    });
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_smart_object_rotate(
    photoshopapi_c_layer* layer, double angle_degrees)
{
    if (!layer) return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    return invoke_v2(layer->last_error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        with_layer(*layer, [&](auto& native)
        {
            using sample_type = typename std::remove_reference_t<decltype(native)>::element_type::value_type;
            require_smart_object<sample_type>(native)->rotate(angle_degrees);
        });
    });
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_smart_object_scale(
    photoshopapi_c_layer* layer, double x_factor, double y_factor)
{
    if (!layer) return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    return invoke_v2(layer->last_error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        with_layer(*layer, [&](auto& native)
        {
            using sample_type = typename std::remove_reference_t<decltype(native)>::element_type::value_type;
            auto smart_object = require_smart_object<sample_type>(native);
            smart_object->scale({x_factor, y_factor}, {smart_object->center_x(), smart_object->center_y()});
        });
    });
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_smart_object_reset_transform(
    photoshopapi_c_layer* layer)
{
    if (!layer) return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    return invoke_v2(layer->last_error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        with_layer(*layer, [&](auto& native)
        {
            using sample_type = typename std::remove_reference_t<decltype(native)>::element_type::value_type;
            require_smart_object<sample_type>(native)->reset_transform();
        });
    });
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_smart_object_reset_warp(
    photoshopapi_c_layer* layer)
{
    if (!layer) return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    return invoke_v2(layer->last_error, PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR, [&]
    {
        with_layer(*layer, [&](auto& native)
        {
            using sample_type = typename std::remove_reference_t<decltype(native)>::element_type::value_type;
            require_smart_object<sample_type>(native)->reset_warp();
        });
    });
}

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_write_ex(
    photoshopapi_c_document* document,
    const char* utf8_path,
    uint8_t force_overwrite)
{
    if (!document || !document->state || !utf8_path || utf8_path[0] == '\0')
    {
        return PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT;
    }
    return invoke_v2(document->last_error, PHOTOSHOPAPI_C_STATUS_WRITE_FAILED, [&]
    {
        auto& state = *document->state;
        if (state.write_attempted) throw std::runtime_error("document has already been consumed by a write attempt");
        state.write_attempted = true;
        const auto path = std::filesystem::u8path(utf8_path);
        if (state.value)
        {
            auto native = std::move(state.value);
            psapi_document_type::write(std::move(*native), path, force_overwrite != 0);
        }
        else if (state.value16)
        {
            auto native = std::move(state.value16);
            psapi_document16_type::write(std::move(*native), path, force_overwrite != 0);
        }
        else if (state.value32)
        {
            auto native = std::move(state.value32);
            psapi_document32_type::write(std::move(*native), path, force_overwrite != 0);
        }
        else
        {
            throw std::runtime_error("document is no longer usable");
        }
    });
}

} // extern "C"
