#pragma once

#include <PhotoshopAPI.h>
#include <LayeredFile/LayerTypes/AdjustmentLayer.h>
#include <LayeredFile/LayerTypes/ArtboardLayer.h>
#include <LayeredFile/LayerTypes/SectionDividerLayer.h>
#include <LayeredFile/LayerTypes/ShapeLayer.h>
#include <LayeredFile/LayerTypes/TextLayer/TextLayer.h>

#include <memory>
#include <string>

namespace psapi = NAMESPACE_PSAPI;

using psapi_document_type = psapi::LayeredFile<psapi::bpp8_t>;
using psapi_document16_type = psapi::LayeredFile<psapi::bpp16_t>;
using psapi_document32_type = psapi::LayeredFile<psapi::bpp32_t>;
using psapi_layer_type = psapi::Layer<psapi::bpp8_t>;
using psapi_layer16_type = psapi::Layer<psapi::bpp16_t>;
using psapi_layer32_type = psapi::Layer<psapi::bpp32_t>;
using psapi_group_type = psapi::GroupLayer<psapi::bpp8_t>;

struct photoshopapi_c_document_state
{
    std::unique_ptr<psapi_document_type> value;
    std::unique_ptr<psapi_document16_type> value16;
    std::unique_ptr<psapi_document32_type> value32;
    bool write_attempted = false;
};

struct photoshopapi_c_document
{
    std::shared_ptr<photoshopapi_c_document_state> state;
    std::string last_error;
};

struct photoshopapi_c_layer
{
    std::shared_ptr<psapi_layer_type> value;
    std::shared_ptr<psapi_layer16_type> value16;
    std::shared_ptr<psapi_layer32_type> value32;
    std::string last_error;
    std::weak_ptr<photoshopapi_c_document_state> document;
    bool attached = false;
};
