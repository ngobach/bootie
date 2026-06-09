#ifndef SCHRIFT_H
#define SCHRIFT_H 1

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SFT_DOWNWARD_Y 0x01

typedef struct SFT          SFT;
typedef struct SFT_Font     SFT_Font;
typedef uint_least32_t      SFT_UChar;
typedef uint_fast32_t       SFT_Glyph;
typedef struct SFT_LMetrics SFT_LMetrics;
typedef struct SFT_GMetrics SFT_GMetrics;
typedef struct SFT_Kerning  SFT_Kerning;
typedef struct SFT_Image    SFT_Image;

struct SFT
{
	SFT_Font *font;
	double    xScale;
	double    yScale;
	double    xOffset;
	double    yOffset;
	int       flags;
};

struct SFT_LMetrics
{
	double ascender;
	double descender;
	double lineGap;
};

struct SFT_GMetrics
{
	double advanceWidth;
	double leftSideBearing;
	int    yOffset;
	int    minWidth;
	int    minHeight;
};

struct SFT_Kerning
{
	double xShift;
	double yShift;
};

struct SFT_Image
{
	void *pixels;
	int   width;
	int   height;
};

SFT_Font *sft_loadmem(const void *mem, size_t size);
void      sft_freefont(SFT_Font *font);

int sft_lmetrics(const SFT *sft, SFT_LMetrics *metrics);
int sft_lookup  (const SFT *sft, SFT_UChar codepoint, SFT_Glyph *glyph);
int sft_gmetrics(const SFT *sft, SFT_Glyph glyph, SFT_GMetrics *metrics);
int sft_kerning (const SFT *sft, SFT_Glyph leftGlyph, SFT_Glyph rightGlyph,
                 SFT_Kerning *kerning);
int sft_render  (const SFT *sft, SFT_Glyph glyph, SFT_Image img);

#ifdef __cplusplus
}
#endif

#endif
