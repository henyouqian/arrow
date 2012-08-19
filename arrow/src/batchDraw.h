#ifndef __BATCH_DRAW_H__
#define __BATCH_DRAW_H__

void batchDrawBegin();
void batchDrawEnd();

void batchCube(float x, float y, float w, float h, float angle, const lw::Color& color);
void batchTriangle(float x1, float y1, float x2, float y2, float x3, float y3, const lw::Color& color);

#endif //__BATCH_DRAW_H__