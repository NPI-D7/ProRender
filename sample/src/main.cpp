/*  ____            __ _                   _   _           _
 *  / ___|_ __ __ _ / _| |_ _   _ ___      | \ | | _____  _| |_
 * | |   | '__/ _` | |_| __| | | / __|_____|  \| |/ _ \ \/ / __|
 * | |___| | | (_| |  _| |_| |_| \__ \_____| |\  |  __/>  <| |_
 *  \____|_|  \__,_|_|  \__|\__,_|___/     |_| \_|\___/_/\_\\__|
 *
 *  _   _ ____ ___      ____ _____ ______ _____
 * | \ | |  _ \_ _|    |  _ \___  / /  _ \___  |_   ____  __
 * |  \| | |_) | |_____| | | | / / /| | | | / /\ \ / /\ \/ /
 * | |\  |  __/| |_____| |_| |/ / / | |_| |/ /  \ V /  >  <
 * |_| \_|_|  |___|    |____//_/_/  |____//_/    \_/  /_/\_\
 * Copyright (C) 2022-2023 Tobi-D7, RSDuck, Onixiya, D7vx-Dev, NPI-D7
 */
#include <stdio.h>
#include <stdlib.h>

#include <3ds.h>
#include <citro3d.h>

#include <prorender.hpp>

int main() {
  gfxInitDefault();
  // consoleInit(GFX_BOTTOM, NULL);
  romfsInit();
  C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
  ProRender::Init();
  C2D_Image app_icon = ProRender::LoadImageFile("romfs:/icon.png");

  int posx = 0, posy = 0;
  bool invert[2] = {false, false};

  while (aptMainLoop()) {
    // Moveing like DVD xd
    if (invert[0])
      posx--;
    else
      posx++;
    if (invert[1])
      posy--;
    else
      posy++;
    if (posx + app_icon.subtex->width > 400)
      invert[0] = true;
    if (posx < 0)
      invert[0] = false;
    if (posy + app_icon.subtex->height > 240)
      invert[1] = true;
    if (posy < 0)
      invert[1] = false;

    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    ProRender::NewFrame();
    ProRender::StartDrawOn(ProRender::Top);
    ProRender::DrawIRect(0, 0, 400, 240, ProRender::FastColorHex("#111111"),
                         ProRender::FastColorHex("#111111"),
                         ProRender::FastColorHex("#222222"),
                         ProRender::FastColorHex("#222222"));
    ProRender::DrawText("This is an example of ProRender!", 0.7f, 5, 5,
                        ProRender::FastColorHex("#ffffff"));
    ProRender::DrawImage(app_icon, posx, posy);
    ProRender::StartDrawOn(ProRender::Bottom);
    ProRender::DrawIRect(0, 0, 400, 240, ProRender::FastColorHex("#222222"),
                         ProRender::FastColorHex("#222222"),
                         ProRender::FastColorHex("#333333"),
                         ProRender::FastColorHex("#333333"));
    C3D_FrameEnd(0);
  }

  ProRender::Exit();
  return 0;
}