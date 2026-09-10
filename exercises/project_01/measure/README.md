# Rectangle measurements across four calibrations

From the repository root:

```sh
cmake --preset debug
cmake --build --preset debug --target measure
./build/debug/measure --compare exercises/project_01 measurement_results 0
```

Use the camera ID of the camera used for calibration (replace `0` if needed).
The comparison mode reads all four `exp*_parameters.xml` files and requests their
image resolution. Frames with a different resolution are rejected.
Choose a new output directory for each session; existing directories are never overwritten.

1. Fix the camera, focus, zoom, lighting, and measurement surface.
2. Put the flat 9 × 6 internal-corner checkerboard, with 25 mm squares, on the
   measurement plane. Press **P**. The same frame establishes four separate plane poses.
3. Keep the camera and surface fixed. Remove the board if necessary and place
   the 75 × 75 mm square near the center and the 125 × 125 mm square farther
   toward the edges. The measured corners must be at the checkerboard plane's
   height: raised object tops require a reference board at that same height.
4. Press **C** to freeze one image containing both objects.
5. Click the four corners of the **75 mm** square in perimeter order, then the
   four corners of the **125 mm** square. Start with the edge you call width;
   proceed clockwise or counterclockwise. Right-click undoes a point; **R** resets all.
6. Press **S** to save. **Esc** cancels.

The raw image clicks are reused for all four calibrations. Each model separately
undistorts the points and maps them through its own plane homography. Width is
 the mean of edges 1–2 and 3–4; height is the mean of edges 2–3 and 4–1.

The output folder contains:

- `measurements.csv`: eight rows (four experiments × two objects), measured and
  real dimensions, absolute errors in millimeters, and absolute percentage errors.
- `plane.png`, `scene.png`, and `selected_corners.png`: source images and click evidence.
- `selected_points.yml`: the eight raw image pixel coordinates, small square first.
- Four separate `*_plane.yml` poses and copies of the four calibration XML files.

To measure previously saved images without opening the camera, use images from
one unchanged setup at the calibration resolution:

```sh
./build/debug/measure --compare exercises/project_01 measurement_repeat 0 \
  measurement_results/plane.png measurement_results/scene.png
```

This recomputes the four plane poses and lets you select corners again.
Never combine a plane image and scene captured after the camera moved.

The original single-calibration mode remains available with its positional arguments:

```sh
./build/debug/measure calibration.xml plane.yml 0 9 6 25
```

Use comparison mode for the report's controlled four-calibration measurements.
