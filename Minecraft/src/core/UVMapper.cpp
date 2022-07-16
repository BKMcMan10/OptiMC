#include "core/UVMapper.h"

/*
	struct TextureFormat
	{
		// UV's are stored in bottom-right, top-right, top-left, bottom-left format
		glm::vec2 uvs[4];
		uint16 id;
		const Texture* texture;
	};
*/

Minecraft::TextureFormat* UVMapper::CreateUV(glm::vec3 size, glm::vec2 textureOffset, static const Minecraft::Texture* texture)
{
	/*using vec2 = glm::vec2;
	vec2 uvs[4] = { vec2{0,0}, vec2{8,0}, vec2{0,8}, vec2{8,8} };
	uint16 id = 250000;
	Minecraft::TextureFormat* texture_format = new Minecraft::TextureFormat;
	//(uvs, id, texture);

	texture_format->id = id;
	texture_format->texture = texture;
	for (int i = 0; i < 4; ++i)
	{
		texture_format->uvs[i] = uvs[i];
	}

	return texture_format;*/
	//Minecraft::TextureFormat* texturef;


	
	return nullptr;

}