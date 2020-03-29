#pragma once
struct IImage {
	enum FlipMode {
		NONE=0,
		HORIZONTAL,
		VERTICAL,
	};

	/*** Unload image data from memory and destroy object */
	virtual void destroy() = 0;

	/**
	* Draw image
	* x, y : screen position in pixels
	* rotation : rotation angle in degrees
	* flip : flip image along axis
	*/
	virtual void draw(int x, int y, float rotation = 0.0f, FlipMode flip = FlipMode::NONE) = 0;

	/** returns image width */
	virtual int width() const = 0;
	/** returns image height */
	virtual int height() const = 0;
};

struct IFont {
	struct Color {
		Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
		: r(r)
		, g(g)
		, b(b)
		, a(a) 
		{}

		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char a;
	};

	/*** Unload font data from memory and destroy object */
	virtual void destroy() = 0;

	/*** Draw text message */
	virtual void draw(int x, int y, const char *text, Color text_color) = 0;
};

struct ISBZLibrary {
	// Some common SDL keycodes
	enum KeyCode {
		KC_RIGHT=0x4000004F,	// SDLK_RIGHT
		KC_LEFT=0x40000050, 	// SDLK_LEFT
		KC_DOWN=0x40000051,		// SDLK_DOWN
		KC_UP=0x40000052,		// SDLK_UP
		KC_ESC=0x1B,			// SDLK_ESCAPE
	};

	/*** Unload library from memory and destroy object */
	virtual void destroy() = 0;

	/**
	* Initialize library
	* width : window width
	* height : window height 
	*/
	virtual void init(int width, int height) = 0;

	/*** Update
	* Call once per frame.
	* It handles the present to screen logic, keyboard events and timer. 
	* returns false if the program window was closed.
	*/
	virtual bool update() = 0;

	/*** Time in seconds since library was initialized */
	virtual float time() = 0;

	/**
	* Fill array with currently pressed keys
	* keys : array pointer
	* size : size of array
	* returns the amount of keys added to the array
	*/
	virtual int pressed_keys(int *keys, int size) = 0;
	virtual bool is_down(int key) = 0;

	/*** Loads new image */
	virtual IImage *load_image(const char *filename) = 0;

	/*** Loads new font */
	virtual IFont *load_font(const char *filename, int point_size=24) = 0;
};
