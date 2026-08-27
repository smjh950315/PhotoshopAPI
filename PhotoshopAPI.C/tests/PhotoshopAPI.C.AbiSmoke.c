#include <PhotoshopAPI.C/PhotoshopAPI.C.h>

int main(void)
{
    photoshopapi_c_document* document = 0;
    photoshopapi_c_layer* group = 0;
    photoshopapi_c_layer* image = 0;
    photoshopapi_c_rgba8_view view = {0, 0, 0, 0};
    photoshopapi_c_layer_options options = {0, 0, 0, 1.0f, 1, 0, {0, 0}};

    (void)photoshopapi_c_get_abi_version();
    (void)photoshopapi_c_document_create(1, 1, &document);
    (void)photoshopapi_c_group_layer_create(&options, &group);
    (void)photoshopapi_c_image_layer_create_rgba8(&view, &options, &image);
    photoshopapi_c_layer_destroy(image);
    photoshopapi_c_layer_destroy(group);
    photoshopapi_c_document_destroy(document);
    return 0;
}
