#ifndef MODEL_H
#define MODEL_H

#include "renderer/Texture.h"
#include "renderer/Shader.h"
#include "renderer/Camera.h"
#include "renderer/Renderer.h"

class Model
{
public:

	void AddBox(Minecraft::TextureType type, static const Minecraft::TextureFormat* texture, glm::vec3 size, glm::vec3 position, glm::vec2 textureSize, glm::vec2 texturePosition, glm::vec3 GlobalPosition);

	void Draw(Minecraft::Shader& shader, Minecraft::Camera& camera);

private:

};

#endif
