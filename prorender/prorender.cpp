/**____            ____                _
 *|  _ \ _ __ ___ |  _ \ ___ _ __   __| | ___ _ __
 *| |_) | '__/ _ \| |_) / _ \ '_ \ / _` |/ _ \ '__|
 *|  __/| | | (_) |  _ <  __/ | | | (_| |  __/ |
 *|_|   |_|  \___/|_| \_\___|_| |_|\__,_|\___|_|
 *
 * _   _ ____ ___      ____ _____
 *| \ | |  _ \_ _|    |  _ \___  |
 *|  \| | |_) | |_____| | | | / /
 *| |\  |  __/| |_____| |_| |/ /
 *|_| \_|_|  |___|    |____//_/
 *
 *  C2D Render Helper
 *  Copyright (C) 2023 NPI-D7
 */

#if __cplusplus < 201703L
#error "You need at least C++17"
#endif

#include <prorender.hpp>

#include <filesystem>
#include <functional>
#include <map>

#ifdef PRO_DEFINE_STB_IMAGE
#if PRO_DEFINE_STB_IMAGE == 1
// Define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#endif

#include <stb_image.h>

/// Lookup Table for Hex colors
static const std::map<char, int> LOOKUP_HEX_COLOR = {
    {'0', 0},  {'1', 1},  {'2', 2},  {'3', 3},  {'4', 4},  {'5', 5},
    {'6', 6},  {'7', 7},  {'8', 8},  {'9', 9},  {'a', 10}, {'b', 11},
    {'c', 12}, {'d', 13}, {'e', 14}, {'f', 15}, {'A', 10}, {'B', 11},
    {'C', 12}, {'D', 13}, {'E', 14}, {'F', 15}};

/// ImageLoader
static unsigned int GetPower2(unsigned int v) {
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;
  return (v >= 64 ? v : 64);
}

static void ConvertImage(C3D_Tex *tex, Tex3DS_SubTexture *subtex,
                         unsigned char *buf, unsigned int size,
                         unsigned int width, unsigned int height,
                         GPU_TEXCOLOR format) {
  // RGBA -> ABGR
  for (unsigned int row = 0; row < width; row++) {
    for (unsigned int col = 0; col < height; col++) {
      unsigned int z = (row + col * width) * 4;

      unsigned char r = *(unsigned char *)(buf + z);
      unsigned char g = *(unsigned char *)(buf + z + 1);
      unsigned char b = *(unsigned char *)(buf + z + 2);
      unsigned char a = *(unsigned char *)(buf + z + 3);

      *(buf + z) = a;
      *(buf + z + 1) = b;
      *(buf + z + 2) = g;
      *(buf + z + 3) = r;
    }
  }

  unsigned int w_pow2 = GetPower2(width);
  unsigned int h_pow2 = GetPower2(height);

  subtex->width = (u16)width;
  subtex->height = (u16)height;
  subtex->left = 0.0f;
  subtex->top = 1.0f;
  subtex->right = (width / (float)w_pow2);
  subtex->bottom = 1.0 - (height / (float)h_pow2);

  C3D_TexInit(tex, (u16)w_pow2, (u16)h_pow2, format);
  C3D_TexSetFilter(tex, GPU_NEAREST, GPU_NEAREST);

  unsigned int pixel_size = (size / width / height);

  memset(tex->data, 0, tex->size);

  for (unsigned int x = 0; x < width; x++) {
    for (unsigned int y = 0; y < height; y++) {
      unsigned int dst_pos =
          ((((y >> 3) * (w_pow2 >> 3) + (x >> 3)) << 6) +
           ((x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | ((y & 2) << 2) |
            ((x & 4) << 2) | ((y & 4) << 3))) *
          pixel_size;
      unsigned int src_pos = (y * width + x) * pixel_size;

      memcpy(&((unsigned char *)tex->data)[dst_pos],
             &((unsigned char *)buf)[src_pos], pixel_size);
    }
  }

  C3D_TexFlush(tex);

  tex->border = 0x00000000;
  C3D_TexSetWrap(tex, GPU_CLAMP_TO_BORDER, GPU_CLAMP_TO_BORDER);
  linearFree(buf);
}

static C2D_Image privLoadImageFile(std::string path) {
  C2D_Image img;
  int w, h, c;
  unsigned char *buffer =
      (unsigned char *)stbi_load(path.c_str(), &w, &h, &c, 4);

  if (w > 1024 || h > 1024) {

    stbi_image_free(buffer);
    return C2D_Image();
  }

  C3D_Tex *tex = new C3D_Tex;
  Tex3DS_SubTexture *subtex = new Tex3DS_SubTexture;
  ConvertImage(tex, subtex, buffer, (unsigned int)(w * h * 4), (unsigned int)w,
               (unsigned int)h, GPU_RGBA8);
  img.tex = tex;
  img.subtex = subtex;
  stbi_image_free(buffer);
  return img;
}

static C2D_Image privLoadImageBuffer(std::vector<unsigned char> file_buffer) {
  C2D_Image img;
  int w, h, c;
  unsigned char *buffer = (unsigned char *)stbi_load_from_memory(
      file_buffer.data(), file_buffer.size(), &w, &h, &c, 4);

  if (w > 1024 || h > 1024) {
    stbi_image_free(buffer);
    return C2D_Image();
  }

  C3D_Tex *tex = new C3D_Tex;
  Tex3DS_SubTexture *subtex = new Tex3DS_SubTexture;
  ConvertImage(tex, subtex, buffer, (unsigned int)(w * h * 4), (unsigned int)w,
               (unsigned int)h, GPU_RGBA8);
  img.tex = tex;
  img.subtex = subtex;
  stbi_image_free(buffer);
  return img;
}

struct RenderContext {
  ~RenderContext() {
    C2D_TextBufDelete(TextBuffer);
    C2D_Fini();
  }

  C3D_RenderTarget *targets[3];

  C2D_TextBuf TextBuffer;
  C2D_Font DefaultFont;

  bool IsTopNow = false;
};

RenderContext *pr_context = NULL;

namespace ProRender {
void Init() {
  pr_context = new RenderContext;
  C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
  C2D_Prepare();
  pr_context->targets[0] = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
  pr_context->targets[1] = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
  pr_context->targets[2] = C2D_CreateScreenTarget(GFX_TOP, GFX_RIGHT);
  pr_context->TextBuffer = C2D_TextBufNew(4096);
  pr_context->DefaultFont = C2D_FontLoadSystem(CFG_REGION_USA);
}

void Exit() { delete pr_context; }

void ClearTextBuffer() { C2D_TextBufClear(pr_context->TextBuffer); }

void NewFrame() {
  C2D_TargetClear(pr_context->targets[0], 0x00000000);
  C2D_TargetClear(pr_context->targets[1], 0x00000000);
  C2D_TargetClear(pr_context->targets[2], 0x00000000);
  ClearTextBuffer();
}

void StartDrawOn(RenderTarget target) {
  // Just for Security
  C3D_FrameBegin(2);

  C2D_Prepare();
  C2D_SceneBegin(pr_context->targets[(int)target]);
  pr_context->IsTopNow = ((target == Top || target == TopRight) ? true : false);
}

unsigned int FastColor32(unsigned char r, unsigned char g, unsigned char b,
                         unsigned char a) {
  return ((((r)&0xFF) << 0) | (((g)&0xFF) << 8) | (((b)&0xFF) << 16) |
          (((a)&0xFF) << 24));
}

unsigned int FastColorF(float r, float g, float b, float a) {
  unsigned char lr = (unsigned char)(r * (float)255);
  unsigned char lg = (unsigned char)(g * (float)255);
  unsigned char lb = (unsigned char)(b * (float)255);
  unsigned char la = (unsigned char)(a * (float)255);
  return ((((lr)&0xFF) << 0) | (((lg)&0xFF) << 8) | (((lb)&0xFF) << 16) |
          (((la)&0xFF) << 24));
}

unsigned int FastColorHex(std::string hex_str, unsigned char a) {
  if (hex_str.length() < 7 ||
      std::find_if(hex_str.begin() + 1, hex_str.end(),
                   [](char c) { return !std::isxdigit(c); }) != hex_str.end()) {
    return ((((0) & 0xFF) << 0) | (((0) & 0xFF) << 8) | (((0) & 0xFF) << 16) |
            (((0) & 0xFF) << 24));
  }

  int r =
      LOOKUP_HEX_COLOR.at(hex_str[1]) * 16 + LOOKUP_HEX_COLOR.at(hex_str[2]);
  int g =
      LOOKUP_HEX_COLOR.at(hex_str[3]) * 16 + LOOKUP_HEX_COLOR.at(hex_str[4]);
  int b =
      LOOKUP_HEX_COLOR.at(hex_str[5]) * 16 + LOOKUP_HEX_COLOR.at(hex_str[6]);
  return ((((r)&0xFF) << 0) | (((g)&0xFF) << 8) | (((b)&0xFF) << 16) |
          (((a)&0xFF) << 24));
}

C2D_Font LoadFont(std::string path) { return C2D_FontLoad(path.c_str()); }

void DeleteFont(C2D_Font font) { C2D_FontFree(font); }

C2D_Image LoadImageFile(std::string path) {
  return privLoadImageFile(path.c_str());
}

C2D_Image LoadImageBuffer(std::vector<unsigned char> buffer) {
  return privLoadImageBuffer(buffer);
}

void GetTextSize(std::string text, float size, float *width, float *height,
                 C2D_Font fnt) {
  C2D_Text c2d_text;
  if (fnt != nullptr)
    C2D_TextFontParse(&c2d_text, fnt, pr_context->TextBuffer, text.c_str());
  else
    C2D_TextFontParse(&c2d_text, pr_context->DefaultFont,
                      pr_context->TextBuffer, text.c_str());
  C2D_TextGetDimensions(&c2d_text, size, size, width, height);
}

float GetTextWidth(std::string text, float size, C2D_Font fnt) {
  float width = 0;
  if (fnt != nullptr)
    GetTextSize(text, size, &width, nullptr, fnt);
  else
    GetTextSize(text, size, &width, nullptr, pr_context->DefaultFont);
  return width;
}

float GetTextHeight(std::string text, float size, C2D_Font fnt) {
  float height = 0;
  if (fnt != nullptr)
    GetTextSize(text, size, nullptr, &height, fnt);
  else
    GetTextSize(text, size, nullptr, &height, pr_context->DefaultFont);
  return height;
}

void DrawRect(float x, float y, float w, float h, unsigned int color) {
  C2D_DrawRectSolid(x, y, 0.5f, w, h, color);
}

void DrawImage(C2D_Image img, float x, float y, float sx, float sy) {
  C2D_DrawImageAt(img, x, y, 0.5f, nullptr, sx, sy);
}

void DrawImageRotated(C2D_Image img, float a, float x, float y, float sx,
                      float sy) {
  C2D_DrawImageAtRotated(img, x, y, 0.5, a, nullptr, sx, sy);
}

void DrawCircle(float x, float y, float r, unsigned int color) {
  C2D_DrawCircleSolid(x, y, 0.5, r, color);
}

void DrawEllipse(float x, float y, float w, float h, unsigned int color) {
  C2D_DrawEllipseSolid(x, y, 0.5, w, h, color);
}

void DrawIRect(float x, float y, float w, float h, unsigned int clr0,
               unsigned int clr1, unsigned int clr2, unsigned int clr3) {
  C2D_DrawRectangle(x, y, 0.5, w, h, clr0, clr1, clr2, clr3);
}

void DrawICircle(float x, float y, float r, unsigned int clr0,
                 unsigned int clr1, unsigned int clr2, unsigned int clr3) {
  C2D_DrawCircle(x, y, 0.5, r, clr0, clr1, clr2, clr3);
}

void DrawIEllipse(float x, float y, float w, float h, unsigned int clr0,
                  unsigned int clr1, unsigned int clr2, unsigned int clr3) {
  C2D_DrawEllipse(x, y, 0.5, w, h, clr0, clr1, clr2, clr3);
}

void DrawTriangle(float x0, float y0, unsigned int clr0, float x1, float y1,
                  unsigned int clr1, float x2, float y2, unsigned int clr2) {
  C2D_DrawTriangle(x0, y0, clr0, x1, y1, clr1, x2, y2, clr2, 0.5);
}

void DrawText(std::string text, float size, float x, float y,
              unsigned int color, float maxW, float maxH, C2D_Font fnt) {
  C2D_Text c2d_text;

  if (fnt != nullptr) {
    C2D_TextFontParse(&c2d_text, fnt, pr_context->TextBuffer, text.c_str());
  } else {
    C2D_TextFontParse(&c2d_text, pr_context->DefaultFont,
                      pr_context->TextBuffer, text.c_str());
  }

  C2D_TextOptimize(&c2d_text);

  float heightScale;
  if (maxH == 0) {
    heightScale = size;
  } else {
    if (fnt != nullptr) {
      heightScale = std::min(
          size, size * (maxH / ProRender::GetTextHeight(text, size, fnt)));
    } else {
      heightScale =
          std::min(size, size * (maxH / ProRender::GetTextHeight(text, size)));
    }
  }

  if (maxW == 0) {
    C2D_DrawText(&c2d_text, C2D_WithColor, x, y, 0.5f, size, heightScale,
                 color);
  } else {
    if (fnt != nullptr) {
      C2D_DrawText(&c2d_text, C2D_WithColor, x, y, 0.5f,
                   std::min(size, size * (maxW / ProRender::GetTextWidth(
                                                     text, size, fnt))),
                   heightScale, color);
    } else {
      C2D_DrawText(
          &c2d_text, C2D_WithColor, x, y, 0.5f,
          std::min(size, size * (maxW / ProRender::GetTextWidth(text, size))),
          heightScale, color);
    }
  }
}

void DrawTextCentered(std::string text, float size, float x, float y,
                      unsigned int color, float maxW, float maxH,
                      C2D_Font fnt) {
  float lineHeight, widthScale;

  // Check for the lineHeight.
  if (fnt != nullptr) {
    lineHeight = ProRender::GetTextHeight(" ", size, fnt);
  } else {
    lineHeight = ProRender::GetTextHeight(" ", size);
  }

  int line = 0;
  while (text.find('\n') != text.npos) {
    if (maxW == 0) {
      // Do the widthScale.
      if (fnt != nullptr) {
        widthScale =
            ProRender::GetTextWidth(text.substr(0, text.find('\n')), size, fnt);
      } else {
        widthScale =
            ProRender::GetTextWidth(text.substr(0, text.find('\n')), size);
      }
    } else {
      // Do the widthScale 2.
      if (fnt != nullptr) {
        widthScale = std::min((float)maxW,
                              ProRender::GetTextWidth(
                                  text.substr(0, text.find('\n')), size, fnt));
      } else {
        widthScale = std::min(
            (float)maxW,
            ProRender::GetTextWidth(text.substr(0, text.find('\n')), size));
      }
    }
    if (fnt != nullptr) {
      ProRender::DrawText(text.substr(0, text.find('\n')), size,
                          (pr_context->IsTopNow ? 200 : 160) + x -
                              (widthScale / 2),
                          y + (lineHeight * line), color, maxW, maxH, fnt);
    } else {
      ProRender::DrawText(text.substr(0, text.find('\n')), size,
                          (pr_context->IsTopNow ? 200 : 160) + x -
                              (widthScale / 2),
                          y + (lineHeight * line), color, maxW, maxH);
    }

    text = text.substr(text.find('\n') + 1);
    line++;
  }

  if (maxW == 0) {
    // Do the next WidthScale.
    if (fnt != nullptr) {
      widthScale =
          ProRender::GetTextWidth(text.substr(0, text.find('\n')), size, fnt);
    } else {
      widthScale =
          ProRender::GetTextWidth(text.substr(0, text.find('\n')), size);
    }
  } else {
    // And again.
    if (fnt != nullptr) {
      widthScale = std::min(
          (float)maxW,
          ProRender::GetTextWidth(text.substr(0, text.find('\n')), size, fnt));
    } else {
      widthScale = std::min(
          (float)maxW,
          ProRender::GetTextWidth(text.substr(0, text.find('\n')), size));
    }
  }
  if (fnt != nullptr) {
    ProRender::DrawText(text.substr(0, text.find('\n')), size,
                        (pr_context->IsTopNow ? 200 : 160) + x -
                            (widthScale / 2),
                        y + (lineHeight * line), color, maxW, maxH, fnt);
  } else {
    ProRender::DrawText(text.substr(0, text.find('\n')), size,
                        (pr_context->IsTopNow ? 200 : 160) + x -
                            (widthScale / 2),
                        y + (lineHeight * line), color, maxW, maxH);
  }
}

void DrawTextRight(std::string text, float size, float x, float y,
                   unsigned int color, float maxW, float maxH, C2D_Font fnt) {
  ProRender::DrawText(text, size,
                      (pr_context->IsTopNow ? 400 : 320) -
                          ProRender::GetTextWidth(text, size, fnt) - x,
                      y, color, maxW, maxH, fnt);
}

void DrawTextWBG(std::string text, float size, float x, float y,
                 unsigned int color, unsigned int bgcolor, float maxW,
                 float maxH, C2D_Font fnt) {
  ProRender::DrawRect(x, y, ProRender::GetTextWidth(text, size),
                      ProRender::GetTextHeight(text, size), bgcolor);
  ProRender::DrawText(text, size, x, y, color, maxW, maxH, fnt);
}

void DrawTextRightWBG(std::string text, float size, float x, float y,
                      unsigned int color, unsigned int bgcolor, float maxW,
                      float maxH, C2D_Font fnt) {
  ProRender::DrawRect((pr_context->IsTopNow ? 400 : 320) -
                          ProRender::GetTextWidth(text, size, fnt) - x,
                      y, ProRender::GetTextWidth(text, size),
                      ProRender::GetTextHeight(text, size), bgcolor);
  ProRender::DrawText(text, size,
                      (pr_context->IsTopNow ? 400 : 320) -
                          ProRender::GetTextWidth(text, size, fnt) - x,
                      y, color, maxW, maxH, fnt);
}
} // namespace ProRender