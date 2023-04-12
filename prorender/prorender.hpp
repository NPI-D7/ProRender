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
 * Version R1
 */

#pragma once
// Config
#define PRO_DEFINE_STB_IMAGE 1

// cxx includes
#include <string>
#include <vector>

// 3ds includes
#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>

namespace ProRender {
enum RenderTarget {
  Top = 0,     //< Top aka TopLeft
  Bottom = 1,  //< Bottom
  TopRight = 2 //< TopRight
};
// Base
void Init();
void Exit();
void ClearTextBuffer();
void NewFrame();
void StartDrawOn(RenderTarget target);

// FastColor
unsigned int FastColor32(unsigned char r, unsigned char g, unsigned char b,
                         unsigned char a = 255);
unsigned int FastColorF(float r, float g, float b, float a = 1.0f);
unsigned int FastColorHex(std::string hex_str, unsigned char a = 255);

// FontLoading
C2D_Font LoadFont(std::string path);
void DeleteFont(C2D_Font font);

// Image Loading
C2D_Image LoadImageFile(std::string path);
C2D_Image LoadImageBuffer(std::vector<unsigned char> buffer);

// TextSizeFunctions
void GetTextSize(std::string text, float size, float *width, float *height,
                 C2D_Font fnt = nullptr);
float GetTextWidth(std::string text, float size, C2D_Font fnt = nullptr);
float GetTextHeight(std::string text, float size, C2D_Font fnt = nullptr);

// Drawing
void DrawRect(float x, float y, float w, float h, unsigned int color);
void DrawImage(C2D_Image img, float x = 0, float y = 0, float sx = 1.0f,
               float sy = 1.0f);
void DrawImageRotated(C2D_Image img, float a, float x = 0, float y = 0,
                      float sx = 1.0f, float sy = 1.0f);
void DrawCircle(float x, float y, float r, unsigned int color);
void DrawEllipse(float x, float y, float w, float h, unsigned int color);
void DrawIRect(float x, float y, float w, float h, unsigned int clr0,
               unsigned int clr1, unsigned int clr2, unsigned int clr3);
void DrawICircle(float x, float y, float r, unsigned int clr0,
                 unsigned int clr1, unsigned int clr2, unsigned int clr3);
void DrawIEllipse(float x, float y, float w, float h, unsigned int clr0,
                  unsigned int clr1, unsigned int clr2, unsigned int clr3);
void DrawTriangle(float x0, float y0, unsigned int clr0, float x1, float y1,
                  unsigned int clr1, float x2, float y2, unsigned int clr2);

// TextDrawing
void DrawText(std::string text, float size, float x, float y,
              unsigned int color, float maxW = 0, float maxH = 0,
              C2D_Font fnt = nullptr);
void DrawTextCentered(std::string text, float size, float x, float y,
                      unsigned int color, float maxW = 0, float maxH = 0,
                      C2D_Font fnt = nullptr);
void DrawTextRight(std::string text, float size, float x, float y,
                   unsigned int color, float maxW = 0, float maxH = 0,
                   C2D_Font fnt = nullptr);

// Extras
void DrawTextWBG(std::string text, float size, float x, float y,
                 unsigned int color, unsigned int bgcolor, float maxW = 0,
                 float maxH = 0, C2D_Font fnt = nullptr);
void DrawTextRightWBG(std::string text, float size, float x, float y,
                      unsigned int color, unsigned int bgcolor, float maxW = 0,
                      float maxH = 0, C2D_Font fnt = nullptr);
} // namespace ProRender