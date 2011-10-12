#include "geometry3D.h"
#include <assert.h>
#include <iostream>
#include <fstream>

geometry3D::geometry3D(std::string verticesFileName, std::string indicesFileName)
{
	// 
	std::ifstream verticesIF(verticesFileName.c_str());
	
	verticesIF >> verticesNum ;
	vertices = new float[3* verticesNum];
	for( int i = 0; i< verticesNum; i++)
	{		
		verticesIF >> vertices[i*3] >> vertices[i*3 + 1] >> vertices[i*3+2];
		//if (i == verticesNum -1)
		//	std::cout << vertices[i*3] << vertices[i*3 + 1] << vertices[i*3+2]<< std::endl;
	}
	verticesIF.close();

	std::ifstream indicesIF(indicesFileName.c_str());
	indicesIF >> triangleNum;
	indices = new unsigned int[3 * triangleNum];
	for( int i = 0; i<triangleNum; i++)
	{
		indicesIF >> indices[i*3] >> indices[i*3 + 1] >> indices[i*3+2];		
	}
	indicesIF.close();

	calcDataRange();

	modelMatrix = glm::mat4x4(1.0f,0.0f,0.0f,0.0f,
							  0.0f,1.0f,0.0f,0.0f,
							  0.0f,0.0f,1.0f,0.0f,
							  0.0f,0.0f,0.0f,1.0f);
	
}


void geometry3D::calcDataRange()
{
	double sumX = 0, sumY =0, sumZ = 0;
	dataRange.minData.x = dataRange.maxData.x = vertices[0];
	dataRange.minData.y = dataRange.maxData.y = vertices[1];
	dataRange.minData.z = dataRange.maxData.z = vertices[2];
	for(int i = 0; i<verticesNum; i++)
	{
		dataRange.minData.x = (dataRange.minData.x < vertices[i*3])? dataRange.minData.x:vertices[i*3];
		dataRange.maxData.x = (dataRange.maxData.x > vertices[i*3])? dataRange.maxData.x:vertices[i*3];
		dataRange.minData.y = (dataRange.minData.y < vertices[i*3+1])? dataRange.minData.y:vertices[i*3+1];
		dataRange.maxData.y = (dataRange.maxData.y > vertices[i*3+1])? dataRange.maxData.y:vertices[i*3+1];
		dataRange.minData.z = (dataRange.minData.z < vertices[i*3+2])? dataRange.minData.z:vertices[i*3+2];
		dataRange.maxData.z = (dataRange.maxData.z > vertices[i*3+2])? dataRange.maxData.z:vertices[i*3+2];
		sumX += vertices[i*3]; sumY += vertices[i*3+1]; sumZ += vertices[i*3+2];
	}
	dataRange.center.x = sumX/verticesNum;
	dataRange.center.y = sumY/verticesNum;
	dataRange.center.z = sumZ/verticesNum;

}