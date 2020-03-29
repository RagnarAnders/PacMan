#include "font.h"
#include <SDL_ttf.h>

Font::Font(TTF_Font *font, SDL_Renderer *renderer) 
:_font(font) 
,_renderer(renderer)
{
}

void Font::destroy() {	
	delete this;
}

void Font::draw(int x, int y, const char *text, Color color) {
	SDL_Color sdl_color = {color.r, color.g, color.b, color.a};
	SDL_Surface *surface = TTF_RenderText_Blended(_font, text, sdl_color);
	SDL_Texture *texture = SDL_CreateTextureFromSurface(_renderer, surface);
	SDL_Rect dst = {x, y, surface->w, surface->h};
	SDL_FreeSurface(surface);	
	SDL_RenderCopy(_renderer, texture, nullptr, &dst);
	SDL_DestroyTexture(texture);
}

IFont* Font::load_font(const char *filename, int size, SDL_Renderer *renderer) {
	TTF_Font *font = TTF_OpenFont(filename, size);
	if (font == nullptr)
		return nullptr;
	return new Font(font, renderer);
}
