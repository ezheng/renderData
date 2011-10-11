#ifndef VIRTUALVIEW_H
#define VIRTUALVIEW_H

#include <glm\glm.hpp>
#include "cameraView.h"

class virtualView
{
public:
	virtualView(cameraView* cam);
	void updateModelViewProjMatrix(float angleUpDown, float angleLeftRight,  glm::vec3 objectCenter);
	void updateModelViewProjMatrix(int sizeDir, glm::vec3 objectCenter);
	void virtualView::updateModelViewProjMatrix(glm::vec3 objectCenter, int isEnlarge);

public:
	

	glm::vec3 optCenterPos;	
	glm::vec3 lookAtPos;
	glm::vec3 upDir;
	glm::vec3 origOptCenterPos;
	
	int imgWidth;
	int imgHeight;

	glm::mat3x3 K;
	float fov;

	glm::mat4x4 modelViewMatrix;
	glm::mat4x4 projMatrix;
	glm::mat4x4 rowMajoredModelViewMatrix;
	glm::mat4x4 rowMajoredProjMatrix;
	glm::mat4x4 modelViewProjMatrix;
	glm::mat4x4 rowMajoredModelViewProjMatrix;

	glm::mat4x4 transformation;

};



#endif