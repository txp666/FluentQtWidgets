# README screenshots

The images in `docs/images/readme/` and the Gallery tour in `docs/images/gallery/` are rendered by
two programs, off screen, so that they come out the same on every run:

- `galleryshots` opens the Gallery (its sources in `examples/gallery`) and saves one page, with any
  open menu or picker painted over it, or the frames of the tour.
- `showcase` builds the "device console" window shown at the top of the README.

Both are Qt 6 only and are not built by default.

## Build

```sh
cmake -S . -B build/shots -G Ninja -DCMAKE_BUILD_TYPE=Release -DFQW_BUILD_EXAMPLES=OFF -DFQW_BUILD_SCREENSHOTS=ON
cmake --build build/shots --target galleryshots showcase
```

Add `-DCMAKE_PREFIX_PATH=...` as for any other build when Qt is not on the default path.

## Render

From the repository root. `offscreen.json` gives the offscreen platform a 3200 x 2000 screen at 96 DPI,
large enough for the 1280 x 800 window and its popups.

```sh
export QT_QPA_PLATFORM=offscreen:configfile=tools/screenshots/offscreen.json
B=build/shots/tools/screenshots
O=docs/images/readme
for lang in zh_CN en; do
    if [ "$lang" = en ]; then s=-en; else s=; fi
    $B/galleryshots $lang home       $O/gallery-home$s.jpg
    $B/galleryshots $lang charts     $O/charts$s.jpg
    $B/galleryshots $lang datepicker $O/date-picker$s.jpg
    $B/galleryshots $lang menu       $O/menu$s.jpg
    $B/showcase     $lang light      $O/showcase-light$s.jpg
    $B/showcase     $lang dark       $O/showcase-dark$s.jpg
done
```

The tour GIF (macOS, which has the GIF encoder in ImageIO):

```sh
$B/galleryshots en frames build/shots/frames
swift tools/screenshots/encode_gif.swift build/shots/frames docs/images/gallery/gallery-tour.gif
```

The fonts come from the system, so images rendered on another platform differ in their text; the
committed ones were rendered on macOS.
