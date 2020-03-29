#pragma once

#include <sbzwhakman.h>
#include <SDL.h>

class Image : public IImage {
public:
	void destroy() override;

	void draw(int x, int y, float rotation, FlipMode mode) override;

	int width() const override {
		return _width;
	}
	int height() const override {
		return _height;
	}

	static IImage *load_image(const char *filename, SDL_Renderer *renderer);
private:
	Image(SDL_Texture *texture, SDL_Renderer *_renderer);

	int			_width;
	int			_height;
	SDL_Texture	 *_texture;
	SDL_Renderer *_renderer;
};
