#include "virtualView.h"
#include <glm\gtc\matrix_transform.hpp>


virtualView::virtualView(cameraView *cam, geometry3D *objModel)
{
	//---------------------------------------------------
	objCenterPos = objModel->dataRange.center;
	optCenterPos = cam->optCenterPos;
	lookAtPos = objCenterPos;		// the lookat position is different from cam->lookAtPos.
	upDir = cam->upDir;
	K = cam->K;

	//_____________________________________
	imgWidth = cam->imgWidth, imgHeight = cam->imgHeight;
	if(imgWidth > 1224 || imgHeight>1024)
		imgWidth /= 2, imgHeight /= 2;	
	//______________________________________
	updataMatrix();		// reculate all the projection matrix.


}

void virtualView::updataMatrix()
{
	modelViewMatrix = glm::lookAt(optCenterPos,lookAtPos, upDir);
	rowMajoredModelViewMatrix = glm::transpose(modelViewMatrix);
	
	float near1 = 0.1f;  float far1 = 100.0;
	float bottom = -( ((float)imgHeight  - K[2][1])/K[1][1] ) * near1 ;
	float top    = ( K[2][1]/K[1][1] ) * near1 ;
	float left   = -( K[2][0]/K[0][0] ) * near1 ;
	float right	 = ( ((float)imgWidth - K[2][0])/K[0][0] ) * near1 ;
	projMatrix = glm::frustum(left,right,bottom,top,near1,far1);

	rowMajoredProjMatrix = glm::transpose(projMatrix);
	modelViewProjMatrix = projMatrix*modelViewMatrix;
	rowMajoredModelViewProjMatrix = glm::transpose(modelViewProjMatrix);
}

virtualView::virtualView(cameraView* cam)
{
	/*modelViewMatrix = cam->modelViewMatrix;
	rowMajoredModelViewMatrix = cam->rowMajoredModelViewMatrix;
	projMatrix = cam->projMatrix;
	rowMajoredProjMatrix = cam->rowMajoredProjMatrix;
	modelViewProjMatrix = cam->modelViewProjMatrix;
	rowMajoredModelViewProjMatrix = cam->rowMajoredModelViewProjMatrix;*/
	transformation = glm::mat4(1.0f);

	
	upDir = cam->upDir;
	optCenterPos = cam->optCenterPos;
	origOptCenterPos = cam->optCenterPos;
	lookAtPos = cam->lookAtPos;
	K = cam->K;

	imgWidth = 1224;
	imgHeight = 1024;

	modelViewMatrix = glm::lookAt(optCenterPos,lookAtPos, upDir);
	rowMajoredModelViewMatrix = glm::transpose(modelViewMatrix);
	fov = cam->fov;


	float near1 = 0.1f;  float far1 = 100.0;
	float bottom = -( ((float)imgHeight  - K[2][1])/K[1][1] ) * near1 ;
	float top    = ( K[2][1]/K[1][1] ) * near1 ;
	float left   = -( K[2][0]/K[0][0] ) * near1 ;
	float right	 = ( ((float)imgWidth - K[2][0])/K[0][0] ) * near1 ;
	projMatrix = glm::frustum(left,right,bottom,top,near1,far1);

	//projMatrix =  glm::perspective(fov, float(imgWidth)/float(imgHeight), 0.1f,2.0f);
	rowMajoredProjMatrix = glm::transpose(projMatrix);
	modelViewProjMatrix = projMatrix*modelViewMatrix;
	rowMajoredModelViewProjMatrix = glm::transpose(modelViewProjMatrix);	

}




void virtualView::updateModelViewProjMatrix(float angleUpDown, float angleLeftRight, glm::vec3 objectCenter)
{
	upDir = glm::normalize(upDir);
	glm::vec3 viewingDir = glm::normalize(lookAtPos - origOptCenterPos);
	glm::vec3 rightDir = glm::cross(viewingDir, upDir);
	glm::vec3 axis = rightDir * glm::vec3(angleUpDown) + upDir * glm::vec3(angleLeftRight);
	float mag = sqrt(angleUpDown*angleUpDown + angleLeftRight * angleLeftRight);
	axis = glm::normalize(axis);
	

	glm::mat4 newTransform = glm::translate(glm::mat4(1), objectCenter);
	//transform = glm::rotate(transform, angleLeftRight, upDir);	
	//transform = glm::rotate(transform, angleUpDown, rightDir);	
	newTransform = glm::rotate(newTransform, mag, axis);
	newTransform = glm::translate(newTransform, -objectCenter);

	transformation = newTransform * transformation;

	modelViewProjMatrix = projMatrix*modelViewMatrix*transformation;	
	rowMajoredModelViewProjMatrix = glm::transpose(modelViewProjMatrix);

	// 
	glm::mat4 newTransformOptCenterPos = glm::translate(glm::mat4(1),objectCenter);
	newTransformOptCenterPos = glm::rotate(newTransformOptCenterPos, -mag, axis);
	newTransformOptCenterPos = glm::translate(newTransformOptCenterPos, -objectCenter);
	glm::vec4 temp = newTransformOptCenterPos * glm::vec4(optCenterPos,1);
	optCenterPos = glm::vec3(temp.x/temp.w, temp.y/temp.w, temp.z/temp.w);

}

void virtualView::updateModelViewProjMatrix(int dir, glm::vec3 objectCenter)
{
	upDir = glm::normalize(upDir);
	glm::vec3 viewingDir = glm::normalize(lookAtPos - optCenterPos);
	glm::vec3 rightDir = glm::cross(viewingDir, upDir);


	float dx = lookAtPos[0] - optCenterPos[0];
	float dy = lookAtPos[1] - optCenterPos[1];
	float dz = lookAtPos[2] - optCenterPos[2];
	float d = std::sqrt(dx * dx + dy * dy + dz * dz);
	glm::mat4x4 newTransform;
	glm::mat4x4 newTransformOptCenterPos;
	switch(dir)
	{
		case 0:		
			{
				newTransform = glm::translate( glm::mat4(1.0f), 1.0f/d/10* rightDir);
				newTransformOptCenterPos = glm::translate(glm::mat4(1.0f), -1.0f/d/10 * rightDir);
				break;
			}
		case 1:		
			{
				newTransform = glm::translate( glm::mat4(1.0f), 1.0f/d/10* upDir);
				newTransformOptCenterPos = glm::translate(glm::mat4(1.0f),-1.0f/d/10*upDir);
				break;
			}
					
		case 2:		
			{
				newTransform = glm::translate( glm::mat4(1.0f), 1.0f/d/10* (-1.0f) * rightDir);
				newTransformOptCenterPos = glm::translate( glm::mat4(1.0f), 1.0f/d/10 *rightDir);
				break;
			}
		case 3:
			{
				newTransform = glm::translate( glm::mat4(1.0f), 1.0f/d/10* (-1.0f) * upDir);
				newTransformOptCenterPos = glm::translate( glm::mat4(1.0f), 1.0f/d/10 *upDir);
				break;
			}
	}	
	

	transformation = newTransform * transformation;
	modelViewProjMatrix = projMatrix*modelViewMatrix*transformation;
	rowMajoredModelViewProjMatrix = glm::transpose(modelViewProjMatrix);

	//
	glm::vec4 temp = newTransformOptCenterPos * glm::vec4(optCenterPos,1);
	optCenterPos = glm::vec3(temp.x/temp.w, temp.y/temp.w, temp.z/temp.w);
}


void virtualView::updateModelViewProjMatrix(glm::vec3 objectCenter, int isEnlarge)
{
	
	glm::mat4 newTransform(1.0f);
	glm::mat4 newTransformOptCenterPos(1.0f);
	float scale = 1.1f;
	if(isEnlarge)
	{
		newTransform = glm::scale(newTransform, glm::vec3(scale));
		newTransformOptCenterPos = glm::scale(newTransformOptCenterPos, glm::vec3(1.0f/scale));
	}		
	else
	{
		newTransform = glm::scale(newTransform, glm::vec3(1/scale));
		newTransformOptCenterPos = glm::scale(newTransformOptCenterPos, glm::vec3(scale));
	}

	glm::mat4 trans1 = glm::translate(glm::mat4(1), objectCenter);
	glm::mat4 trans2 = glm::translate(glm::mat4(1), -objectCenter);
	transformation = (trans1* newTransform* trans2) * transformation;
	modelViewProjMatrix = projMatrix*modelViewMatrix*transformation;
	rowMajoredModelViewProjMatrix = glm::transpose(modelViewProjMatrix);

	
	glm::vec4 temp = newTransformOptCenterPos * glm::vec4(optCenterPos,1);
	optCenterPos = glm::vec3(temp.x/temp.w, temp.y/temp.w, temp.z/temp.w);

}