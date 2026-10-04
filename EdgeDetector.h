#pragma once
#include <string>

#ifdef AICADO_HAS_OPENCV
// Canny edge detection on an image (e.g. a part photo). Writes the edge image to outPath and
// returns the number of contours found, or -1 if the image cannot be read.
int detectEdges(const std::string& imagePath, const std::string& outPath);
#endif
