#pragma once

#include <stdint.h>

#if defined(PHOTOSHOPAPI_C_EXPORTS)
#    if defined(_WIN32)
#        define PHOTOSHOPAPI_C_API __declspec(dllexport)
#    else
#        define PHOTOSHOPAPI_C_API
#    endif
#else
#    if defined(_WIN32)
#        define PHOTOSHOPAPI_C_API __declspec(dllimport)
#    else
#        define PHOTOSHOPAPI_C_API
#    endif
#endif

#if defined(_WIN32)
#    define PHOTOSHOPAPI_C_CALL __cdecl
#else
#    define PHOTOSHOPAPI_C_CALL
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#define PHOTOSHOPAPI_C_ABI_VERSION 3u

typedef enum photoshopapi_c_status
{
    PHOTOSHOPAPI_C_STATUS_SUCCESS = 0,
    PHOTOSHOPAPI_C_STATUS_INVALID_ARGUMENT = 1,
    PHOTOSHOPAPI_C_STATUS_DIMENSIONS_INVALID = 2,
    PHOTOSHOPAPI_C_STATUS_DIMENSIONS_MISMATCH = 3,
    PHOTOSHOPAPI_C_STATUS_ALLOCATION_FAILED = 4,
    PHOTOSHOPAPI_C_STATUS_OWNERSHIP_ERROR = 5,
    PHOTOSHOPAPI_C_STATUS_IO_ERROR = 6,
    PHOTOSHOPAPI_C_STATUS_WRITE_FAILED = 7,
    PHOTOSHOPAPI_C_STATUS_NOT_SUPPORTED = 8,
    PHOTOSHOPAPI_C_STATUS_BUFFER_TOO_SMALL = 9,
    PHOTOSHOPAPI_C_STATUS_TYPE_MISMATCH = 10,
    PHOTOSHOPAPI_C_STATUS_READ_FAILED = 11,
    PHOTOSHOPAPI_C_STATUS_INTERNAL_ERROR = 12
} photoshopapi_c_status;

typedef enum photoshopapi_c_bit_depth
{
    PHOTOSHOPAPI_C_BIT_DEPTH_8 = 8,
    PHOTOSHOPAPI_C_BIT_DEPTH_16 = 16,
    PHOTOSHOPAPI_C_BIT_DEPTH_32 = 32
} photoshopapi_c_bit_depth;

/* Values follow the Photoshop file-format color-mode identifiers. */
typedef enum photoshopapi_c_color_mode
{
    PHOTOSHOPAPI_C_COLOR_MODE_BITMAP = 0,
    PHOTOSHOPAPI_C_COLOR_MODE_GRAYSCALE = 1,
    PHOTOSHOPAPI_C_COLOR_MODE_INDEXED = 2,
    PHOTOSHOPAPI_C_COLOR_MODE_RGB = 3,
    PHOTOSHOPAPI_C_COLOR_MODE_CMYK = 4,
    PHOTOSHOPAPI_C_COLOR_MODE_MULTICHANNEL = 7,
    PHOTOSHOPAPI_C_COLOR_MODE_DUOTONE = 8,
    PHOTOSHOPAPI_C_COLOR_MODE_LAB = 9
} photoshopapi_c_color_mode;

typedef enum photoshopapi_c_layer_type
{
    PHOTOSHOPAPI_C_LAYER_TYPE_UNKNOWN = 0,
    PHOTOSHOPAPI_C_LAYER_TYPE_IMAGE = 1,
    PHOTOSHOPAPI_C_LAYER_TYPE_GROUP = 2,
    PHOTOSHOPAPI_C_LAYER_TYPE_TEXT = 3,
    PHOTOSHOPAPI_C_LAYER_TYPE_SMART_OBJECT = 4,
    PHOTOSHOPAPI_C_LAYER_TYPE_SHAPE = 5,
    PHOTOSHOPAPI_C_LAYER_TYPE_ADJUSTMENT = 6,
    PHOTOSHOPAPI_C_LAYER_TYPE_ARTBOARD = 7,
    PHOTOSHOPAPI_C_LAYER_TYPE_SECTION_DIVIDER = 8
} photoshopapi_c_layer_type;

typedef enum photoshopapi_c_blend_mode
{
    PHOTOSHOPAPI_C_BLEND_MODE_PASSTHROUGH = 0,
    PHOTOSHOPAPI_C_BLEND_MODE_NORMAL = 1,
    PHOTOSHOPAPI_C_BLEND_MODE_DISSOLVE = 2,
    PHOTOSHOPAPI_C_BLEND_MODE_DARKEN = 3,
    PHOTOSHOPAPI_C_BLEND_MODE_MULTIPLY = 4,
    PHOTOSHOPAPI_C_BLEND_MODE_COLOR_BURN = 5,
    PHOTOSHOPAPI_C_BLEND_MODE_LINEAR_BURN = 6,
    PHOTOSHOPAPI_C_BLEND_MODE_DARKER_COLOR = 7,
    PHOTOSHOPAPI_C_BLEND_MODE_LIGHTEN = 8,
    PHOTOSHOPAPI_C_BLEND_MODE_SCREEN = 9,
    PHOTOSHOPAPI_C_BLEND_MODE_COLOR_DODGE = 10,
    PHOTOSHOPAPI_C_BLEND_MODE_LINEAR_DODGE = 11,
    PHOTOSHOPAPI_C_BLEND_MODE_LIGHTER_COLOR = 12,
    PHOTOSHOPAPI_C_BLEND_MODE_OVERLAY = 13,
    PHOTOSHOPAPI_C_BLEND_MODE_SOFT_LIGHT = 14,
    PHOTOSHOPAPI_C_BLEND_MODE_HARD_LIGHT = 15,
    PHOTOSHOPAPI_C_BLEND_MODE_VIVID_LIGHT = 16,
    PHOTOSHOPAPI_C_BLEND_MODE_LINEAR_LIGHT = 17,
    PHOTOSHOPAPI_C_BLEND_MODE_PIN_LIGHT = 18,
    PHOTOSHOPAPI_C_BLEND_MODE_HARD_MIX = 19,
    PHOTOSHOPAPI_C_BLEND_MODE_DIFFERENCE = 20,
    PHOTOSHOPAPI_C_BLEND_MODE_EXCLUSION = 21,
    PHOTOSHOPAPI_C_BLEND_MODE_SUBTRACT = 22,
    PHOTOSHOPAPI_C_BLEND_MODE_DIVIDE = 23,
    PHOTOSHOPAPI_C_BLEND_MODE_HUE = 24,
    PHOTOSHOPAPI_C_BLEND_MODE_SATURATION = 25,
    PHOTOSHOPAPI_C_BLEND_MODE_COLOR = 26,
    PHOTOSHOPAPI_C_BLEND_MODE_LUMINOSITY = 27
} photoshopapi_c_blend_mode;

typedef enum photoshopapi_c_layer_color
{
    PHOTOSHOPAPI_C_LAYER_COLOR_NONE = 0,
    PHOTOSHOPAPI_C_LAYER_COLOR_RED = 1,
    PHOTOSHOPAPI_C_LAYER_COLOR_ORANGE = 2,
    PHOTOSHOPAPI_C_LAYER_COLOR_YELLOW = 3,
    PHOTOSHOPAPI_C_LAYER_COLOR_GREEN = 4,
    PHOTOSHOPAPI_C_LAYER_COLOR_BLUE = 5,
    PHOTOSHOPAPI_C_LAYER_COLOR_VIOLET = 6,
    PHOTOSHOPAPI_C_LAYER_COLOR_GRAY = 7,
    PHOTOSHOPAPI_C_LAYER_COLOR_SEAFOAM = 8,
    PHOTOSHOPAPI_C_LAYER_COLOR_INDIGO = 9,
    PHOTOSHOPAPI_C_LAYER_COLOR_MAGENTA = 10,
    PHOTOSHOPAPI_C_LAYER_COLOR_FUSCHIA = 11
} photoshopapi_c_layer_color;

typedef enum photoshopapi_c_compression
{
    PHOTOSHOPAPI_C_COMPRESSION_RAW = 0,
    PHOTOSHOPAPI_C_COMPRESSION_RLE = 1,
    PHOTOSHOPAPI_C_COMPRESSION_ZIP = 2,
    PHOTOSHOPAPI_C_COMPRESSION_ZIP_PREDICTION = 3
} photoshopapi_c_compression;

typedef struct photoshopapi_c_document photoshopapi_c_document;
typedef struct photoshopapi_c_layer photoshopapi_c_layer;

typedef struct photoshopapi_c_document_info
{
    uint64_t width;
    uint64_t height;
    float dpi;
    photoshopapi_c_bit_depth bit_depth;
    photoshopapi_c_color_mode color_mode;
    uint32_t root_layer_count;
} photoshopapi_c_document_info;

typedef struct photoshopapi_c_layer_info
{
    photoshopapi_c_layer_type type;
    photoshopapi_c_bit_depth bit_depth;
    photoshopapi_c_color_mode color_mode;
    photoshopapi_c_blend_mode blend_mode;
    photoshopapi_c_layer_color display_color;
    uint32_t width;
    uint32_t height;
    float center_x;
    float center_y;
    float opacity;
    float fill;
    uint8_t visible;
    uint8_t locked;
    uint8_t clipping_mask;
    uint8_t reserved;
} photoshopapi_c_layer_info;

typedef struct photoshopapi_c_mask_info
{
    uint64_t width;
    uint64_t height;
    double center_x;
    double center_y;
    double feather;
    uint8_t has_mask;
    uint8_t disabled;
    uint8_t relative_to_layer;
    uint8_t default_color;
    uint8_t has_density;
    uint8_t density;
    uint8_t has_feather;
    uint8_t reserved;
} photoshopapi_c_mask_info;

/* Straight-alpha RGBA8 scanlines. The memory is borrowed for the call. */
typedef struct photoshopapi_c_rgba8_view
{
    const uint8_t* pixels;
    uint32_t width;
    uint32_t height;
    uint32_t stride_bytes;
} photoshopapi_c_rgba8_view;

/* Packed RGB8 scanlines with no alpha channel. The memory is borrowed for the call. */
typedef struct photoshopapi_c_rgb8_view
{
    const uint8_t* pixels;
    uint32_t width;
    uint32_t height;
    uint32_t stride_bytes;
} photoshopapi_c_rgb8_view;

/* One 8-bit grayscale pixel mask. The memory is borrowed for the call. */
typedef struct photoshopapi_c_mask8_view
{
    const uint8_t* pixels;
    uint32_t width;
    uint32_t height;
    uint32_t stride_bytes;
} photoshopapi_c_mask8_view;

/* Coordinates use the document's top-left origin. Name is UTF-8 and borrowed. */
typedef struct photoshopapi_c_layer_options
{
    const char* name;
    int32_t left;
    int32_t top;
    float opacity;
    uint8_t visible;
    uint8_t locked;
    uint8_t reserved[2];
} photoshopapi_c_layer_options;

PHOTOSHOPAPI_C_API uint32_t PHOTOSHOPAPI_C_CALL photoshopapi_c_get_abi_version(void);

/* Document construction and lifetime. Read automatically detects 8/16/32-bit data. */
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_create(
    uint32_t width,
    uint32_t height,
    photoshopapi_c_document** out_document);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_create_ex(
    photoshopapi_c_bit_depth bit_depth,
    photoshopapi_c_color_mode color_mode,
    uint64_t width,
    uint64_t height,
    photoshopapi_c_document** out_document);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_read(
    const char* utf8_path,
    photoshopapi_c_document** out_document);
PHOTOSHOPAPI_C_API void PHOTOSHOPAPI_C_CALL photoshopapi_c_document_destroy(
    photoshopapi_c_document* document);

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_get_info(
    const photoshopapi_c_document* document,
    photoshopapi_c_document_info* out_info);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_set_size(
    photoshopapi_c_document* document,
    uint64_t width,
    uint64_t height);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_set_dpi(
    photoshopapi_c_document* document,
    float dpi);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_set_compression(
    photoshopapi_c_document* document,
    photoshopapi_c_compression compression);
/* Sets the merged RGB8 composite for an 8-bit RGB document. The memory is borrowed for the call. */
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_set_merged_rgb8(
    photoshopapi_c_document* document,
    const photoshopapi_c_rgb8_view* source);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_invalidate_text_cache(
    photoshopapi_c_document* document);

/* Binary output follows the two-call buffer contract and reports bytes. */
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_get_icc_profile(
    const photoshopapi_c_document* document,
    uint8_t* buffer,
    uint64_t buffer_capacity,
    uint64_t* out_required_size);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_set_icc_profile(
    photoshopapi_c_document* document,
    const uint8_t* data,
    uint64_t data_size);

/* Every returned layer is an owning handle that must be destroyed by the caller. */
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_get_root_layer(
    photoshopapi_c_document* document,
    uint32_t index,
    photoshopapi_c_layer** out_layer);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_find_layer(
    photoshopapi_c_document* document,
    const char* utf8_path,
    photoshopapi_c_layer** out_layer);

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_group_layer_create(
    const photoshopapi_c_layer_options* options,
    photoshopapi_c_layer** out_layer);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_image_layer_create_rgba8(
    const photoshopapi_c_rgba8_view* source,
    const photoshopapi_c_layer_options* options,
    photoshopapi_c_layer** out_layer);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_image_layer_create_rgb8(
    const photoshopapi_c_rgb8_view* source,
    const photoshopapi_c_layer_options* options,
    photoshopapi_c_layer** out_layer);
PHOTOSHOPAPI_C_API void PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_destroy(
    photoshopapi_c_layer* layer);

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_get_info(
    const photoshopapi_c_layer* layer,
    photoshopapi_c_layer_info* out_info);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_set_info(
    photoshopapi_c_layer* layer,
    const photoshopapi_c_layer_info* info);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_get_name(
    const photoshopapi_c_layer* layer,
    char* buffer,
    uint32_t buffer_capacity,
    uint32_t* out_required_buffer_size);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_set_name(
    photoshopapi_c_layer* layer,
    const char* utf8_name);

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_get_child_count(
    const photoshopapi_c_layer* layer,
    uint32_t* out_count);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_get_child(
    photoshopapi_c_layer* layer,
    uint32_t index,
    photoshopapi_c_layer** out_child);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_group_get_collapsed(
    const photoshopapi_c_layer* layer,
    uint8_t* out_collapsed);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_group_set_collapsed(
    photoshopapi_c_layer* layer,
    uint8_t collapsed);

/* A layer handle may be released after it is attached; its parent keeps ownership. */
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_add_layer(
    photoshopapi_c_document* document,
    photoshopapi_c_layer* layer);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_remove_layer(
    photoshopapi_c_document* document,
    photoshopapi_c_layer* layer);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_move_layer(
    photoshopapi_c_document* document,
    photoshopapi_c_layer* layer,
    photoshopapi_c_layer* new_parent);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_group_add_layer(
    photoshopapi_c_layer* group,
    photoshopapi_c_layer* child);

/* Channel indices use Photoshop's integer channel IDs (RGB: 0,1,2; alpha: -1; mask: -2). */
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_get_channel_indices(
    photoshopapi_c_layer* layer,
    int32_t* buffer,
    uint32_t buffer_capacity,
    uint32_t* out_required_count);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_get_channel_data(
    photoshopapi_c_layer* layer,
    int32_t channel_index,
    void* buffer,
    uint64_t buffer_capacity,
    uint64_t* out_required_size);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_set_channel_data(
    photoshopapi_c_layer* layer,
    int32_t channel_index,
    const void* data,
    uint64_t data_size);

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_set_mask8(
    photoshopapi_c_layer* layer,
    const photoshopapi_c_mask8_view* mask);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_get_mask_info(
    const photoshopapi_c_layer* layer,
    photoshopapi_c_mask_info* out_info);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_set_mask_info(
    photoshopapi_c_layer* layer,
    const photoshopapi_c_mask_info* info);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_get_mask_data(
    photoshopapi_c_layer* layer,
    void* buffer,
    uint64_t buffer_capacity,
    uint64_t* out_required_size);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_set_mask_data(
    photoshopapi_c_layer* layer,
    const void* data,
    uint64_t data_size,
    uint64_t width,
    uint64_t height);

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_text_layer_get_text(
    const photoshopapi_c_layer* layer,
    char* buffer,
    uint32_t buffer_capacity,
    uint32_t* out_required_buffer_size);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_text_layer_set_text(
    photoshopapi_c_layer* layer,
    const char* utf8_text);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_text_layer_replace_text(
    photoshopapi_c_layer* layer,
    const char* utf8_old_text,
    const char* utf8_new_text,
    uint8_t replace_all);

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_smart_object_get_hash(
    const photoshopapi_c_layer* layer,
    char* buffer,
    uint32_t buffer_capacity,
    uint32_t* out_required_buffer_size);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_smart_object_get_filename(
    const photoshopapi_c_layer* layer,
    char* buffer,
    uint32_t buffer_capacity,
    uint32_t* out_required_buffer_size);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_smart_object_get_filepath(
    const photoshopapi_c_layer* layer,
    char* buffer,
    uint32_t buffer_capacity,
    uint32_t* out_required_buffer_size);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_smart_object_replace(
    photoshopapi_c_layer* layer,
    const char* utf8_path,
    uint8_t link_externally);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_smart_object_move(
    photoshopapi_c_layer* layer,
    double x_offset,
    double y_offset);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_smart_object_rotate(
    photoshopapi_c_layer* layer,
    double angle_degrees);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_smart_object_scale(
    photoshopapi_c_layer* layer,
    double x_factor,
    double y_factor);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_smart_object_reset_transform(
    photoshopapi_c_layer* layer);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_smart_object_reset_warp(
    photoshopapi_c_layer* layer);

/* Writes a PSD and consumes the document. The path is UTF-8. */
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_write(
    photoshopapi_c_document* document,
    const char* utf8_path);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_write_ex(
    photoshopapi_c_document* document,
    const char* utf8_path,
    uint8_t force_overwrite);

/* The returned error is UTF-8. Query the NUL-terminated size with NULL/0. */
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_get_last_error(
    const photoshopapi_c_document* document,
    char* buffer,
    uint32_t buffer_capacity,
    uint32_t* out_required_buffer_size);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_get_last_error(
    const photoshopapi_c_layer* layer,
    char* buffer,
    uint32_t buffer_capacity,
    uint32_t* out_required_buffer_size);

#ifdef __cplusplus
}
#endif
