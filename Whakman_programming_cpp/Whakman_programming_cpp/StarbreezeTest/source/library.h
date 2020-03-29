#pragma once
#include <sbzwhakman.h>
#include <SDL.h>
#include <vector>
#include "utility.h"

class SBZLibrary : public ISBZLibrary {
public:
	SBZLibrary();
	~SBZLibrary();

	void destroy() override;
	void init(int width, int height) override;
	bool update() override;
	float time() override;

	int pressed_keys(int *keys, int size) override;
	bool is_down(int key) override;

	IImage*	load_image(const char *filename) override;
	IFont*	load_font(const char *font, int size);
private:
	bool			_initialized;
	SDL_Window*		_window;
	SDL_Renderer*	_renderer;
	Timer			_timer;
	float			_prev_time;
	std::vector<int> _pressed_keys;
	static int		_ref_counter;
};
