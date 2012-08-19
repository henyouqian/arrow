#include "stdafx.h"
#include "batchDraw.h"

void batchDrawBegin(){
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	cml::Matrix44 m;
#ifdef __APPLE__
	float rotDegree = 0;
	const lw::App::Orientation orient = lw::App::getConfig().orientation;
	switch (orient) {
		case lw::App::ORIENTATION_LEFT:
			rotDegree = 90;
			break;
		case lw::App::ORIENTATION_RIGHT:
			rotDegree = -90;
			break;
		default:
			break;
	}
	glRotatef(rotDegree, 0, 0, 1);
#endif
	const lw::App::Config conf = lw::App::getConfig();
	cml::matrix_orthographic_RH(m, -(float)conf.width*0.5f, (float)conf.width*0.5f, -(float)conf.height*0.5f, (float)conf.height*0.5f, -1000.f, 1000.f, cml::z_clip_neg_one);
	glMultMatrixf(m.data());

	glEnable(GL_BLEND);
}

namespace{
	const int CUBE_MAX_NUM = 1000;
	float vtxPos[CUBE_MAX_NUM*6*2];
	float* pVtxPos = vtxPos;
	GLubyte vtxColor[CUBE_MAX_NUM*6*4];
	GLubyte* pVtxColor = vtxColor;
}

void batchFlush(){
	glVertexPointer(2, GL_FLOAT, 0, vtxPos);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, vtxColor);
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(pVtxPos - vtxPos)/2);
	pVtxPos = vtxPos;
	pVtxColor = vtxColor;
}

void batchDrawEnd(){
	batchFlush();
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}

void batchCube(float x, float y, float w, float h, float angle, const lw::Color& color){
	cml::Matrix33 m;
	cml::matrix_translation_2D(m, x, y);
	cml::Matrix33 m1;
	m1.identity();
	cml::matrix_rotate_2D(m1, angle);
	m *= m1;

	cml::Vector2 v2[6];
	v2[0][0] = -w*.5f;
	v2[0][1] = -h*.5f;
	v2[1][0] = +w*.5f;
	v2[1][1] = -h*.5f;
	v2[2][0] = +w*.5f;
	v2[2][1] = +h*.5f;

	v2[3][0] = -w*.5f;
	v2[3][1] = -h*.5f;
	v2[4][0] = +w*.5f;
	v2[4][1] = +h*.5f;
	v2[5][0] = -w*.5f;
	v2[5][1] = +h*.5f;
	v2[0] = cml::transform_point_2D(m, v2[0]);
	v2[1] = cml::transform_point_2D(m, v2[1]);
	v2[2] = cml::transform_point_2D(m, v2[2]);
	v2[3] = cml::transform_point_2D(m, v2[3]);
	v2[4] = cml::transform_point_2D(m, v2[4]);
	v2[5] = cml::transform_point_2D(m, v2[5]);


	for ( int i = 0; i < 6; ++i ){
		pVtxPos[0] = v2[i][0];
		pVtxPos[1] = v2[i][1];
		pVtxPos += 2;
		pVtxColor[0] = color.r;
		pVtxColor[1] = color.g;
		pVtxColor[2] = color.b;
		pVtxColor[3] = color.a;
		pVtxColor += 4;
	}
	if ( (pVtxPos - vtxPos) >= (CUBE_MAX_NUM-1)*6*2 ){
		batchFlush();
	}
}

void batchTriangle(float x1, float y1, float x2, float y2, float x3, float y3, const lw::Color& color){
	pVtxPos[0] = x1;
	pVtxPos[1] = y1;
	pVtxPos[2] = x2;
	pVtxPos[3] = y2;
	pVtxPos[4] = x3;
	pVtxPos[5] = y3;
	pVtxPos += 6;
	
	pVtxColor[0] = color.r;
	pVtxColor[1] = color.g;
	pVtxColor[2] = color.b;
	pVtxColor[3] = color.a;
	pVtxColor[4] = color.r;
	pVtxColor[5] = color.g;
	pVtxColor[6] = color.b;
	pVtxColor[7] = color.a;
	pVtxColor[8] = color.r;
	pVtxColor[9] = color.g;
	pVtxColor[10] = color.b;
	pVtxColor[11] = color.a;
	pVtxColor += 12;
	if ( (pVtxPos - vtxPos) >= (CUBE_MAX_NUM-1)*6*2 ){
		batchFlush();
	}
}