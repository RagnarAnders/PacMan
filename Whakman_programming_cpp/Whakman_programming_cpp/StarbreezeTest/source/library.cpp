#include <cassert>
#include <SDL.h>
#include <SDL_image.h>
#include "library.h"
#include "image.h"
#include "font.h"

#define W_TITLE	"Whakman"

int SBZLibrary::_ref_counter = 0;
const float k_FRAME_CAP_T = 1.0f / 60.0f;

SBZLibrary::SBZLibrary()
: _window(nullptr)
, _renderer(nullptr)
, _initialized(false)
, _prev_time(0)
{
	assert(_ref_counter == 0);
	++_ref_counter;	
}

SBZLibrary::~SBZLibrary() {
	--_ref_counter;
}

void SBZLibrary::destroy() {
	if (_initialized) {
		SDL_DestroyRenderer(_renderer);
		SDL_DestroyWindow(_window);
		TTF_Quit();
		IMG_Quit();
		SDL_Quit();
	}	
	delete this;
}

void SBZLibrary::init(int width, int height) {	
	SDL_Init(SDL_INIT_VIDEO);
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();
	_window = SDL_CreateWindow(W_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0);
	_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
	SDL_RenderClear(_renderer);
	_initialized = true;
}

int SBZLibrary::pressed_keys(int *keys, int size) {
	auto it = _pressed_keys.begin();
	auto end = _pressed_keys.end();
	int i = 0;

	for(i = 0; it != end && i < size; ++it, ++i) {
		keys[i] = *it;
	}

	return i;
}

bool SBZLibrary::is_down(int key)
{
	for (int k : _pressed_keys)
	{
		if (k == key)
		{
			return true;
		}
	}
	return false;
}

bool SBZLibrary::update() {
	assert(_initialized);

	if (!_initialized)
		return false;
		
	SDL_RenderPresent(_renderer);
	SDL_RenderClear(_renderer);

	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			return false;
		}
		switch(event.type) {
			case SDL_QUIT:
				return false;
			case SDL_KEYDOWN:
				{
					// New keys are added to the end of the vector. This ensures correct order of keypresses.
					auto it = std::find(_pressed_keys.begin(), _pressed_keys.end(), event.key.keysym.sym);
					if (it == _pressed_keys.end()) {
						_pressed_keys.push_back(event.key.keysym.sym);
					}
				}
				break;
			case SDL_KEYUP:
				{
					auto it = std::find(_pressed_keys.begin(), _pressed_keys.end(), event.key.keysym.sym);
					if (it != _pressed_keys.end()) {
						_pressed_keys.erase(it);						
					}
				}
				break;
		}
	}

	_timer.snapshot();

	// Cap framerate
	float cur = _timer.time();
	float dt = cur - _prev_time;
	int wait = static_cast<int>((k_FRAME_CAP_T - dt) * 1000.0f);
	if (wait > 0) {
		SDL_Delay(wait);
		_timer.snapshot();
	}

	_prev_time = _timer.time();

	return true;
}

float SBZLibrary::time() {
	return _timer.time();
}

IImage *SBZLibrary::load_image(const char *filename) {
	if (!_initialized)
		return nullptr;
	return Image::load_image(filename, _renderer);
}

IFont* SBZLibrary::load_font(const char *filename, int size) {
	if (!_initialized)
		return nullptr;
	return Font::load_font(filename, size, _renderer);
}

extern "C" {
__declspec(dllexport) ISBZLibrary *CreateLibrary() {
	return new SBZLibrary();
}
};
