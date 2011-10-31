//===============================================
//			CameraView.h
//			Mingsong Dou (doums@cs.unc.edu)
//			Modified from Li Guan's code
//			August, 2010
//===============================================
#ifndef _CAMERA_VIEW_H_
#define _CAMERA_VIEW_H_

#include <glm\glm.hpp>
#include <string>
#include <fstream>
#include "highgui.h"




class cameraView
{
public:
	cameraView(std::string imgFile, std::string camPosFile, std::string KFile);
	~cameraView()
	{
		if(img!=NULL)
		{cvReleaseImage(&img);}
	}
private:
	void setProjMatrix();
	void setModelViewMatrix();
	void setTextureMatrix();	
	void getCenteredImage();

	
	


	IplImage* origImg;

public:
	IplImage *img;
	glm::mat3x3 K;
	glm::mat3x3 R;
	glm::vec3 T;
	
	glm::mat4x4 projMatrix;
	glm::mat4x4 modelViewMatrix;
	glm::mat4x4 textureMatrix;
	glm::mat4x4 modelViewProjMatrix;

	glm::mat4x4 rowMajoredProjMatrix;
	glm::mat4x4 rowMajoredModelViewMatrix;
	glm::mat4x4 rowMajoredTextureMatrix;
	glm::mat4x4 rowMajoredModelViewProjMatrix;

	glm::vec3 optCenterPos;
	int imgWidth;
	int imgHeight;
	float fov;
	glm::vec3 centerPos;
	glm::vec3 lookAtPos;
	glm::vec3 upDir;		
	

};

#endif