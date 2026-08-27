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

#define PHOTOSHOPAPI_C_ABI_VERSION 1u

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
    PHOTOSHOPAPI_C_STATUS_BUFFER_TOO_SMALL = 9
} photoshopapi_c_status;

typedef struct photoshopapi_c_document photoshopapi_c_document;
typedef struct photoshopapi_c_layer photoshopapi_c_layer;

/* Straight-alpha RGBA8 scanlines. The memory is borrowed for the call. */
typedef struct photoshopapi_c_rgba8_view
{
    const uint8_t* pixels;
    uint32_t width;
    uint32_t height;
    uint32_t stride_bytes;
} photoshopapi_c_rgba8_view;

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

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_create(
    uint32_t width,
    uint32_t height,
    photoshopapi_c_document** out_document);
PHOTOSHOPAPI_C_API void PHOTOSHOPAPI_C_CALL photoshopapi_c_document_destroy(
    photoshopapi_c_document* document);

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_group_layer_create(
    const photoshopapi_c_layer_options* options,
    photoshopapi_c_layer** out_layer);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_image_layer_create_rgba8(
    const photoshopapi_c_rgba8_view* source,
    const photoshopapi_c_layer_options* options,
    photoshopapi_c_layer** out_layer);
PHOTOSHOPAPI_C_API void PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_destroy(
    photoshopapi_c_layer* layer);

/* A layer handle may be released after it is attached; its parent keeps ownership. */
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_add_layer(
    photoshopapi_c_document* document,
    photoshopapi_c_layer* layer);
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_group_add_layer(
    photoshopapi_c_layer* group,
    photoshopapi_c_layer* child);

PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_layer_set_mask8(
    photoshopapi_c_layer* layer,
    const photoshopapi_c_mask8_view* mask);

/* Writes a PSD and consumes the document. The path is UTF-8. */
PHOTOSHOPAPI_C_API photoshopapi_c_status PHOTOSHOPAPI_C_CALL photoshopapi_c_document_write(
    photoshopapi_c_document* document,
    const char* utf8_path);

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
