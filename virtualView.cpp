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
	updataMatrix();		// recaculate all the transformation matrix.


}

void virtualView::updataMatrix()	// after changing optCenterPos, lookAtPos, upDir, K, then update all the transformation matrix
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
	transformation = glm::mat4(1.0f);
	
	upDir = cam->upDir;
	optCenterPos = cam->optCenterPos;
	origOptCenterPos = cam->optCenterPos;
	lookAtPos = cam->lookAtPos;
	K = cam->K;

	//________________________________________
	imgWidth = cam->imgWidth, imgHeight = cam->imgHeight;
	if(imgWidth > 1224 || imgHeight>1024)
		imgWidth /= 2, imgHeight /= 2;	
	//________________________________________

	updataMatrix();
	//fov = cam->fov;
}
