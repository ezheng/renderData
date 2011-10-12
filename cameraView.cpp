#include "cameraView.h"
#include <math.h>
#include <glm\gtc\matrix_transform.hpp>
#include <iostream>
#define PI 3.1415926

cameraView::cameraView(std::string imgFile, std::string camPosFile, std::string KFile)
{
	// read K(intrinsic parameter) 
	std::ifstream inK(KFile.c_str());
	assert(inK.is_open());
	float dataK[5];
	for(unsigned int i = 0; i < 5; i++)
	{
		inK >> dataK[i];
	}	
	K[0][0] = dataK[0], K[0][1] = 0,	K[0][2] = 0;
	K[1][0] = 0,	    K[1][1] = dataK[3], K[1][2] = 0;
	K[2][0] = dataK[2],	K[2][1] = dataK[4],	K[2][2] = 1;

	//read camera pose info
	std::ifstream in(camPosFile.c_str());
	assert(in.is_open() == true);
	float data[12];
	for(unsigned int i = 0; i < 12; i++)
	{
		in>>data[i];
		std::cout << data[i] << " " << std::endl;
	}
	in.close();
	R[0][0] = data[0], R[1][0] = data[1], R[2][0] = data[2];
	R[0][1] = data[4], R[1][1] = data[5], R[2][1] = data[6];
	R[0][2] = data[8], R[1][2] = data[9], R[2][2] = data[10];
	T[0] = data[3], T[1] = data[7], T[2] = data[11];

	// read image
	origImg = cvLoadImage(imgFile.c_str(),1);
	assert(origImg != NULL);
	
	//cvFlip(origImg,0,0);
	//cvFlip(origImg,0,1);
	//getCenteredImage();	
	img = cvCreateImage(cvGetSize(origImg), origImg->depth, origImg->nChannels);
	cvCopy(origImg, img);
	imgWidth = img->width;
	imgHeight = img->height;
	cvReleaseImage(&origImg);
	//cvFlip(img,0,1);
	cvFlip(img);
		
	setModelViewMatrix();
	setProjMatrix();
	setTextureMatrix();
	modelViewProjMatrix = projMatrix * modelViewMatrix ;

	
	/*glm::mat4x4 biasScaleMatrix = glm::mat4x4(0.5f, 0.0f, 0.0f, 0.0f,
											  0.0f, 0.5f, 0.0f, 0.0f,
											  0.0f, 0.0f, 0.5f, 0.0f,
											  0.5f, 0.5f, 0.5f, 1.0f);
	glm::mat4 temp = biasScaleMatrix * modelViewProjMatrix;*/

	rowMajoredModelViewMatrix = glm::transpose(modelViewMatrix);
	rowMajoredProjMatrix = glm::transpose(projMatrix);
	rowMajoredTextureMatrix = glm::transpose(textureMatrix);		
	rowMajoredModelViewProjMatrix = glm::transpose(modelViewProjMatrix);

}

void cameraView::getCenteredImage()
{
	int centerX = (int)floor(K[2][0]);
	int centerY = (int)floor(K[2][1]);
	int origWidth = origImg->width;
	int origHeight = origImg->height;
	int startX, startY;
	if(origWidth - centerX >= centerX)
	{startX = 0, imgWidth = centerX *2 +1;}
	else
	{startX = 2*centerX - origWidth + 1;
	imgWidth = (origWidth-1-centerX)*2+1;}	

	if(origHeight - centerY >= centerY)
	{startY = 0, imgHeight = centerY *2+1;}
	else
	{startY = 2*centerY - origHeight + 1;
	imgHeight = (origHeight-1-centerY)*2+1;}	
	assert(startX>=0 && startY>=0 && startX<imgWidth && startY<imgHeight);
	CvRect r = cvRect(startX, startY, imgWidth, imgHeight);

	cvSetImageROI(origImg, r);
	img = cvCreateImage(cvGetSize(origImg), origImg->depth, origImg->nChannels);
	cvCopy(origImg, img);
	cvResetImageROI(origImg);	
	//cvSaveImage("hello.jpg", img);
	
}


void cameraView::setModelViewMatrix()
{
	glm::vec3 viewDir = glm::vec3(0.0f,0.0f,1.0f);
	viewDir = glm::transpose(R) * viewDir ;	
	
	/*glm::vec3*/ centerPos = -1*glm::transpose(R)*T;
	/*glm::vec3*/ lookAtPos = centerPos + viewDir;
	/*glm::vec3*/ upDir = glm::vec3(0.0f,-1.0f,0.0f);
	upDir = glm::normalize(glm::transpose(R) * upDir);
	
	modelViewMatrix = glm::lookAt(centerPos,lookAtPos, upDir);
	optCenterPos = centerPos;
}

void cameraView::setProjMatrix()
{
	fov = float(atan(imgHeight/2.0f/(K[1][1]))*180.0f/PI*2);
	//projMatrix = glm::perspective(fov, float(imgWidth)/float(imgHeight), 0.1f,2.0f);	
	float near1 = 0.1f;  float far1 = 200.0;
	float bottom = -( ((float) imgHeight  - K[2][1])/K[1][1] ) * near1 ;
	float top    = ( K[2][1]/K[1][1] ) * near1 ;
	float left   = -( K[2][0]/K[0][0] ) * near1 ;
	float right	 = ( ((float)imgWidth - K[2][0])/K[0][0] ) * near1 ;
	projMatrix = glm::frustum(left,right,bottom,top,near1,far1);

	//glFrustum( left, right, bottom, top, near1, far1); 
}

void cameraView::setTextureMatrix()
{
	glm::mat4x4 biasScaleMatrix = glm::mat4x4(0.5f, 0.0f, 0.0f, 0.0f,
											  0.0f, 0.5f, 0.0f, 0.0f,
											  0.0f, 0.0f, 0.5f, 0.0f,
											  0.5f, 0.5f, 0.5f, 1.0f);
	textureMatrix = biasScaleMatrix * projMatrix * modelViewMatrix;
	//textureMatrix = biasScaleMatrix * glm::perspective(10.0f,float(imgWidth)/float(imgHeight), 0.1f,10.0f) * modelViewMatrix;
}
