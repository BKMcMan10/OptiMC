#ifndef UVMAPPER_H
#define UVMAPPER_H

#include "renderer/Renderer.h"
#include "renderer/Texture.h"
#include "world/BlockMap.h"

class UVMapper
{

public:

	Minecraft::TextureFormat* CreateUV(glm::vec3 size, glm::vec2 textureOffset, static const Minecraft::Texture* texture);

private:



};

#endif
