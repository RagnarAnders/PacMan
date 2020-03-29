#pragma once

#include <sbzwhakman.h>
#include <SDL_ttf.h>

class Font : public IFont {
public:
	void destroy() override;
	void draw(int x, int y, const char *text, Color color) override;

	static IFont* load_font(const char *filename, int size, SDL_Renderer *renderer);
private:
	Font(TTF_Font *font, SDL_Renderer *renderer);

	TTF_Font*		_font;
	SDL_Renderer*	_renderer;
};
