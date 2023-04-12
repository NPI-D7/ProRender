# ProRender
Simple C2D Render Helepr

# Usage/Building
Just Copy the prorender Folder into your Project or gitclone it and add it to your makefile.

For the sample you can `cd` into `sample` and run `make` to build.
# Config
If you're getting `multiple definition of stbi*` then you need to go to `prorender.h` and set 
`PRO_DEFINE_STB_IMAGE` to `0`. This should fix the issue
```
// Config
#define PRO_DEFINE_STB_IMAGE 1 // 1 Means Enabled, 0 Disabled
```
# Versions
## R1
Most Minimalist Version of ProRender
Supports Loading pngs/jpgs/bmps etc
Good Textdrawing Engine also with Backround
Simplifyd all C2D_Draw Functions (like no depth)
Configurable stbimage if youre already using in your Project

R1 is Currently the only public Version cause it has no major bugs.

# Cresits
- @Tobi-D7 Main Dev
- @nothings stb_image
- @devkitPro ctru, citro2d, citro3d