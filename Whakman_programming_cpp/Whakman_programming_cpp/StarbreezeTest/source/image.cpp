#include "image.h"
#include <SDL_image.h>

Image::Image(SDL_Texture *texture, SDL_Renderer *renderer)
:_texture(texture)
,_renderer(renderer)
{	
	SDL_QueryTexture(texture, nullptr, nullptr, &_width, &_height);
	SDL_SetTextureBlendMode(texture, SDL_BlendMode::SDL_BLENDMODE_BLEND);
}

void Image::draw(int x, int y, float rotation, FlipMode mode) {	
	SDL_Rect dst = {x, y, _width, _height};
	SDL_Point center = {_width / 2, _height / 2};
	SDL_RendererFlip flip = SDL_RendererFlip::SDL_FLIP_NONE;
	if (mode == FlipMode::HORIZONTAL) {
		flip = SDL_RendererFlip::SDL_FLIP_HORIZONTAL;
	} 
	else if (mode == FlipMode::VERTICAL) {
		flip = SDL_RendererFlip::SDL_FLIP_VERTICAL;
	}
	SDL_RenderCopyEx(_renderer, _texture, nullptr, &dst, rotation, &center, flip);
}

void Image::destroy() {	
	SDL_DestroyTexture(_texture);
	delete this;
}

IImage *Image::load_image(const char *filename, SDL_Renderer *renderer) {
	SDL_Texture *texture = IMG_LoadTexture(renderer, filename);
	if (texture == nullptr)
		return nullptr;	
	return new Image(texture, renderer);
}
