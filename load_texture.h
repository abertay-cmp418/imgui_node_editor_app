#ifndef _LOAD_TEXTURE_H
#define _LOAD_TEXTURE_H

#include <system/platform.h>
#include <graphics/texture.h>

// FUNCTION PROTOTYPES
gef::Texture* CreateTextureFromPNG(const char* png_filename, gef::Platform& platform);

#endif // _LOAD_TEXTURE_H


