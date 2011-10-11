#ifndef GEOMETRY3D_H
#define GEOMETRY3D_H

#include <string>
#include <fstream>
#include <math.h>
#include <glm\glm.hpp>

#define BIGNUM 1000000000

class dataScape
{
public:
	//float minX, minY, minZ;
	//float maxX, maxY, maxZ;
	//float center;
	glm::vec3 minData;
	glm::vec3 maxData;
	glm::vec3 center;
};


class geometry3D
{
private: 
	void calcDataRange();

public:
	geometry3D(std::string filename);	
	geometry3D(std::string verticesFileName, std::string indicesFileName);
	
	~geometry3D()
	{
		delete []vertices;
		delete []indices;
	}
	int triangleNum;
	int verticesNum;
	float* vertices;
	unsigned int* indices;
	dataScape dataRange;
	
	// Model transformation
	float angleUpDown;
	float angleLeftRight;
	
	// 
	glm::mat4x4 modelMatrix;


};

#endif