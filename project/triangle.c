#include "shape.h"

/* Using arrow shape as a base to build triangle check
 */
int abTriCheck(const AbTri *shape, const Vec2 *centerPos, const Vec2 *pixel) {
  Vec2 relPos;
  int row, col, within = 0;
  int size = shape->size;
  int halfSize = size/2, quarterSize = halfSize/2;
  vec2Sub(&relPos, pixel, centerPos); /* vector from center to pixel */
  row = relPos.axes[1]; col = -relPos.axes[0];
  col = (col >= 0) ? col : -col;
  if (row >= 0) {
    if (row <= quarterSize) {
      within = col <= row;
    }
  }
  return within;
}

void abTriGetBounds(const AbTri *shape, const Vec2 *centerPos, Region *bounds) {
  int size = shape->size, halfSize = size / 2;
  bounds->topLeft.axes[0] = centerPos->axes[0];
  bounds->topLeft.axes[1] = centerPos->axes[1] - halfSize;
  bounds->botRight.axes[0] = centerPos->axes[0] + size;
  bounds->botRight.axes[1] = centerPos->axes[1] + halfSize;
}
