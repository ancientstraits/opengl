#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct TextContext {
	FT_Library lib;
	FT_Face face;
};

static TextContext* text_context_create(const char* font_path, int font_size) {
	TextContext* tc = calloc(1, sizeof *tc);

	// Remember that FreeType functions return non-zero on error
	assert(!FT_Init_FreeType(&(tc->lib)));
	assert(!FT_New_Face(tc->lib, font_path, 0, &(tc->face)));
	FT_Set_Pixel_Sizes(tc->face, 0, font_size);

	return tc;
}

int main() {
}

