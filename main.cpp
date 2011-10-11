#include "cameraView.h"
#include "geometry3d.h"
#include "virtualView.h"
#include <math.h>
#include <vector>
#include "globalFunc.h"
#include <gl\glut.h>
#include <gl\gl.h>
#include <cg\cg.h>
#include <cg\cggl.h>
#include <string>
#include <glm\gtc\matrix_transform.hpp>
#include "checkforError.h" 
#include <iostream>

std::vector<geometry3D*> allGeometry;
std::vector<cameraView*> allCamView;
virtualView* virCam;

const unsigned int numOfFile = 18;
int virtualCamWidth = 1224;
int virtualCamHeight = 1024;
const unsigned int numOfCams = 4;
GLuint camImgTexId[numOfCams];
GLuint depthTextureId[numOfCams];
GLuint fboId[numOfCams];
static CGcontext   myCgContext;
static CGprofile   myCgVertexProfile,
                   myCgFragmentProfile;
static CGprogram   myCgVertexProgram,
                   myCgFragmentProgram;
static CGparameter V_modelViewProj,                   
                   V_textureMatrix[numOfCams],					  
				   V_imgTexture[numOfCams],
				   V_shadowTexture[numOfCams],
				   V_virtualCamPos,
				   V_sourceCamPos[numOfCams];
static const char *myVertexProgramFileName = "V_projTex.cg",
				  *myVertexProgramName = "V_projTex",
                  *myFragmentProgramFileName = "F_projTex.cg",
			      *myFragmentProgramName = "F_projTex";
const std::string filePath = ".\\data_enrique\\";
void drawScene();
void initializeCGShaders();
float lastPos[2], lastAngle[2], angleUpDown = 0, angleLeftRight = 0;
int textureSymbolArray[] ={GL_TEXTURE0,GL_TEXTURE1,GL_TEXTURE2,GL_TEXTURE3,
		GL_TEXTURE4,GL_TEXTURE5,GL_TEXTURE6,GL_TEXTURE7};

void generateShadowFBO(int idx, int shadowMapWidth, int shadowMapHeight)
{	
	// Try to use a texture depth component
	glGenTextures(1, &depthTextureId[idx]);
	glBindTexture(GL_TEXTURE_2D, depthTextureId[idx]);	
	// GL_LINEAR does not make sense for depth texture. However, next tutorial shows usage of GL_LINEAR and PCF
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	
	// Remove artefact on the edges of the shadowmap
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );	
	// No need to force GL_DEPTH_COMPONENT24, drivers usually give you the max precision if available 
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);	
	// create a framebuffer object
	glGenFramebuffersEXT(1, &fboId[idx]);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId[idx]);	
	// Instruct openGL that we won't bind a color texture with the currently binded FBO
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);	
	// attach the texture to FBO depth attachment point. attach the depth to texture
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D, depthTextureId[idx], 0);	
	// check FBO status
	GLenum FBOstatus = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if(FBOstatus != GL_FRAMEBUFFER_COMPLETE_EXT)
		std::cout<<"GL_FRAMEBUFFER_COMPLETE_EXT failed, CANNOT use FBO"<<std::endl;	
	// switch back to window-system-provided framebuffer
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void computeDepthMap(int idx)
{	
	unsigned int sourceCamWidth = allCamView[idx]->imgWidth;
	unsigned int sourceCamHeight = allCamView[idx]->imgHeight;
	//First step: Render from the light POV to a FBO, story depth values only
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId[idx]);	//Rendering offscreen	
	//Using the fixed pipeline to render to the depthbuffer
	glUseProgramObjectARB(0);	
	// In the case we render the shadowmap to a higher resolution, the viewport must be modified accordingly.
	glViewport(0,0,sourceCamWidth, sourceCamHeight);
	//glViewport(0,0,cam->m_Width, cam->m_Height);	
	glEnable(GL_DEPTH_TEST);
	// Clear previous frame values
	glClear(GL_DEPTH_BUFFER_BIT);
	//Disable color rendering, we only want to write to the Z-Buffer
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); 
	glBindTexture(GL_TEXTURE_2D, depthTextureId[idx]);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, sourceCamWidth, sourceCamHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	// setting the projection matrix and modelview matrix
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(&(allCamView[idx]->projMatrix[0][0]));
	//glLoadMatrixf(&sourceCamProjection[0][0]);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(&(allCamView[idx]->modelViewMatrix[0][0]));
	//glLoadMatrixf(&sourceCamModelView[0][0]);
	//glCullFace(GL_FRONT);	
	// draw the scene. This is the first pass
	glPolygonOffset(5.0f,5.0f);
	glEnable(GL_POLYGON_OFFSET_FILL);
	drawScene();
	glDisable(GL_POLYGON_OFFSET_FILL);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void drawScene()
{		
	glEnableClientState(GL_VERTEX_ARRAY);	
	for(int i = 0; i<numOfFile; i++)
	{
		glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), allGeometry[i]->vertices);	
		glDrawElements(GL_TRIANGLES, allGeometry[i]->triangleNum*3, GL_UNSIGNED_INT, 
		(unsigned int*)allGeometry[i]->indices);
	}	
	glDisableClientState(GL_VERTEX_ARRAY);
}
void display()
{
	int value;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB,&value);


	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		
	glEnable(GL_DEPTH_TEST);
	
	for(int i = 0; i<numOfCams; i++)
	{
		computeDepthMap(i);		
	}
	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	glEnable(GL_DEPTH_TEST);

	// set the virtual camera pose	
	V_modelViewProj = cgGetNamedParameter( myCgVertexProgram, "modelViewProj") ;                  
	checkForCgError("could not get " "modelViewProj" " parameter");               
	cgSetMatrixParameterfr(V_modelViewProj, &(virCam->rowMajoredModelViewProjMatrix[0][0]));		
	checkForCgError("setting model view matrix");	
	V_virtualCamPos = cgGetNamedParameter(myCgVertexProgram, "virCamPos");
	checkForCgError("could not get " "virtualCamPos" " parameter"); 
	cgSetParameter3fv(V_virtualCamPos, &(virCam->optCenterPos[0]));
	checkForCgError("virtual Cam Position");
	
	// set the shadow texture
	for(int i =0; i<numOfCams; i++)
	{
		//int tempID = i + numOfCams;
		std::stringstream ss;
		ss<<i;		
		//ACTIVATE_TEXTURE(tempID);
		glActiveTexture(textureSymbolArray[i+numOfCams]);
		//glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthTextureId[i]);
		std::string s = std::string("shadowTexture")+ss.str();
		V_shadowTexture[i] = cgGetNamedParameter(myCgFragmentProgram, s.c_str());
		checkForCgError("could not get " "shadowTexture0" " parameter"); 
		cgGLSetTextureParameter(V_shadowTexture[i], depthTextureId[i]);
		checkForCgError("setting shadow texture");
	}

	//glBindTexture(GL_TEXTURE_2D, camImgTexId[0]);
	//cgGLSetTextureParameter(V_shadowTexture, camImgTexId[0]);	
	//glActiveTextureARB(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, camImgTexId[0]);	
	//V_imgTexture = cgGetNamedParameter(myCgFragmentProgram, "imgTexture");
	//checkForCgError("could not get " "imgTexture" " parameter"); 
	//cgGLSetTextureParameter(V_shadow
	//Texture, camImgTexId[0]);
	//checkForCgError("setting image texture");	
	
	cgGLBindProgram(myCgVertexProgram);
    checkForCgError("binding vertex program");
    cgGLEnableProfile(myCgVertexProfile);
    checkForCgError("enabling vertex profile");
	cgGLBindProgram(myCgFragmentProgram);
	checkForCgError("binding fragment program");
    cgGLEnableProfile(myCgFragmentProfile);
    checkForCgError("enabling fragment profile");
	// enable textures
	
	for(int i = 0; i<numOfCams; i++)
	{		
		cgGLEnableTextureParameter(V_imgTexture[i]);
		checkForCgError("enable image texture");			
		cgGLEnableTextureParameter(V_shadowTexture[i]);
		checkForCgError("enable shadow texture");
	}
    glViewport(0,0,virtualCamWidth, virtualCamHeight);
	//glEnable(GL_CULL_FACE);
	drawScene();
	//glFinish();
	/*float* data = new float[sourceCamWidth * sourceCamHeight];
	glReadPixels(0, 0, sourceCamWidth, sourceCamHeight, GL_DEPTH_COMPONENT, GL_FLOAT, data);*/

	glutSwapBuffers();
	
	cgGLDisableProfile(myCgVertexProfile);
	checkForCgError("disabling vertex profile");
	cgGLDisableProfile(myCgFragmentProfile);
	checkForCgError("disabling fragment profile");
	for(int i = 0; i<numOfCams; i++)
	{
		cgGLDisableTextureParameter(V_imgTexture[0]);
		checkForCgError("disabling decal texture");
		cgGLDisableTextureParameter(V_shadowTexture[0]);
		checkForCgError("disabling decal texture");	
	}
	glDisable(GL_DEPTH_TEST);
}

void init()
{
		// init glew functions
	init_glew_and_check_gl_version();	

	glColor3f(1.0,1.0,1.0);
	glClearDepth(1.0f);
	//glDepthFunc(GL_LEQUAL);
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);		
	
	// initialize the geometry and image information
	std::string allFileNames[numOfFile] =  { "submodel-horz-0-00548.wrl",
								"submodel-horz-1-00548.wrl",
								"submodel-horz-2-00548.wrl",
								"submodel-vert-0-00548.wrl",
								"submodel-vert-1-00548.wrl",
								"submodel-vert-2-00548.wrl",
								"submodel-vert-3-00548.wrl",
								"submodel-vert-4-00548.wrl",
								"submodel-vert-5-00548.wrl",
								"submodel-vert-6-00548.wrl",
								"submodel-vert-7-00548.wrl",
								"submodel-vert-8-00548.wrl",
								"submodel-vert-9-00548.wrl",
								"submodel-vert-10-00548.wrl",
								"submodel-vert-11-00548.wrl",
								"submodel-vert-12-00548.wrl",
								"submodel-vert-13-00548.wrl",
								"submodel-vert-14-00548.wrl"};

	
	int allImageIndices[numOfCams] = {1361,964,643,922};
	allGeometry.clear();
	for(int i = 0 ;i<numOfFile; i++)
	{
		//geometry3D *pModel3d = new geometry3D(filePath + "vertical fusion\\vrml\\" + allFileNames[i] );
		//allGeometry.push_back(pModel3d);		
	}	
	std::string verticesFileName = "E:\\data\\bombData_cut\\vertical fusion\\vrml\\vertices.txt";
	std::string indicesFileName = "E:\\data\\bombData_cut\\vertical fusion\\vrml\\indices.txt";	

	geometry3D *object3DModel = new geometry3D( verticesFileName, indicesFileName);

	allCamView.clear();
	for(int i = 0; i<numOfCams; i++)
	{
		char fileNum[9];
		sprintf(fileNum, "%.8d", allImageIndices[i]);
		cameraView *pCamView = new cameraView(
			filePath + "database\\cam-side\\undistorted\\undistorted" + fileNum + ".jpg",
		filePath + "database\\cam-side\\tracker3d\\camera" + fileNum + ".txt",
		filePath + "database\\cam-side\\" + "K.txt");
		allCamView.push_back(pCamView);
	}
	virCam = new virtualView(allCamView[0]);

// ----------------------------------------------------------------------------------
	//generate fbo	
	for(int i = 0 ; i<numOfCams; i++)
	{
		generateShadowFBO(i, allCamView[i]->imgWidth, allCamView[i]->imgHeight);		
	}	

	// read two texture images
	glGenTextures(numOfCams, camImgTexId);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	for(int i = 0 ; i<numOfCams;i++)
	{		 
		//ACTIVATE_TEXTURE(i);
		glActiveTexture(textureSymbolArray[i]);
		//glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, camImgTexId[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 
					  allCamView[i]->imgWidth, allCamView[i]->imgHeight,
					  0, GL_BGR, GL_UNSIGNED_BYTE,
					  allCamView[i]->img->imageData);
	}	

	glBindTexture(GL_TEXTURE_2D, 0);
	if(!initMultitexture())
	{
		std::cout<<"cannot use multiple texutre"<<std::endl;
	}

	// set up shaders
	initializeCGShaders();

}


void initializeCGShaders()
{
	myCgContext = cgCreateContext();
	checkForCgError("creating context");
	cgGLSetDebugMode(CG_FALSE);
	//cgGLSetManageTextureParameters(myCgContext, CG_TRUE);
	//cgSetParameterSettingMode(myCgContext, CG_DEFERRED_PARAMETER_SETTING);  // not sure what this is used for
//------------------------------------------------------------------------------	
	myCgVertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
    cgGLSetOptimalOptions(myCgVertexProfile);
	//std::cout<< cgGetProfileString(cgGLGetLatestProfile(CG_GL_VERTEX))<<std::endl;

    checkForCgError("selecting vertex profile");
	
	myCgVertexProgram =
    cgCreateProgramFromFile(
      myCgContext,              /* Cg runtime context */
      CG_SOURCE,                /* Program in human-readable form */
      myVertexProgramFileName,  /* Name of file containing program */
      myCgVertexProfile,        /* Profile: OpenGL ARB vertex program */
      myVertexProgramName,      /* Entry function name */
      NULL);                    /* No extra compiler options */
	checkForCgError("creating vertex program from file");
	cgGLLoadProgram(myCgVertexProgram);
	checkForCgError("loading vertex program");
	//------------------------------------------------------------------------------

	myCgFragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
	cgGLSetOptimalOptions(myCgFragmentProfile);
	checkForCgError("selecting fragment profile");

	myCgFragmentProgram =
	cgCreateProgramFromFile(
		myCgContext,              /* Cg runtime context */
		CG_SOURCE,                /* Program in human-readable form */
		myFragmentProgramFileName,
		myCgFragmentProfile,      /* Profile: latest fragment profile */
		myFragmentProgramName,    /* Entry function name */
		NULL); /* No extra compiler options */
	checkForCgError("creating fragment program from string");
	cgGLLoadProgram(myCgFragmentProgram);
	checkForCgError("loading fragment program");
//-------------------------------------------------------------------------------
  //set CGparameters
	for(int i = 0; i<numOfCams; i++)
	{
		std::stringstream ss;//create a stringstream
		ss << i;
		std::string s;
		s = std::string("textureMatrix") + ss.str();
		V_textureMatrix[i] = cgGetNamedParameter( myCgVertexProgram,  s.c_str()) ;                  
		checkForCgError("could not get " "textureMatrix" " parameter");            
		cgSetMatrixParameterfr(V_textureMatrix[i], &(allCamView[i] ->rowMajoredTextureMatrix[0][0]));	
		checkForCgError("setting model view matrix");
		s = std::string("imgTexture")+ss.str();
		
		glActiveTexture(textureSymbolArray[i]);
		glBindTexture(GL_TEXTURE_2D, camImgTexId[i]);
		V_imgTexture[i] = cgGetNamedParameter(myCgFragmentProgram,s.c_str());
		checkForCgError("could not get " "image texture0" " parameter"); 
		cgGLSetTextureParameter(V_imgTexture[i], camImgTexId[i]);
		checkForCgError("setting texture0");

		s = std::string("sourceCamPos")+ss.str();
		V_sourceCamPos[i] = cgGetNamedParameter(myCgVertexProgram, s.c_str());
		checkForCgError("could not get " "virtualCamPos" " parameter"); 
		cgSetParameter3fv(V_sourceCamPos[i], &(allCamView[i]->optCenterPos[0]));
		checkForCgError("source Cam Position");
	}
	// depth texture is set later

}


void moveMeFlat(int dir) 
{
	virCam->updateModelViewProjMatrix(dir, allGeometry[0]->dataRange.center);	
}

void mouse(int button, int state, int x, int y)
{
	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		lastPos[0] = x;
		lastPos[1] = y;		
	}	
	
}

void mouseTrack(int x, int y)
{
	
	// get the new angle and position
	int dx = x - lastPos[0];
	int dy = y - lastPos[1];
	angleUpDown = /*lastAngle[0] +*/ (float(dy) * 10 * virCam->fov / virtualCamHeight);
	angleLeftRight = /*lastAngle[1] +*/ (float(dx) * 10 * virCam->fov / virtualCamHeight);
	//angleUpDown = float(dy) * 1.8 * virCam->fov / virtualCamHeight;
	//angleLeftRight = (float(dx) * 1.8 * virCam->fov / virtualCamHeight);

	if(angleLeftRight >= 180)
	angleLeftRight -= 360;
	if(angleLeftRight <= -180)
	angleLeftRight += 360;
	if(angleUpDown <= -180)
	angleUpDown += 360;
	if(angleUpDown >= 180)
	angleUpDown -= 360;

	//update the model matrix
	if(dx!=0 || dy!=0)
	{virCam->updateModelViewProjMatrix(angleUpDown, angleLeftRight, allGeometry[0]->dataRange.center);
	glutPostRedisplay();
	}

	lastPos[0] = x;
	lastPos[1] = y;

}
void inputKey(int key, int x, int y) 
{
	switch (key) {
		case GLUT_KEY_RIGHT : moveMeFlat(0);break;
		case GLUT_KEY_UP : moveMeFlat(1);break;
		case GLUT_KEY_LEFT : moveMeFlat(2); break;
		case GLUT_KEY_DOWN : moveMeFlat(3); break;
		default: break;
	}
	glutPostRedisplay();
}

void keyboard(unsigned char key,int x, int y)
{
	if(key == 'b' || key == 'B')	//enlarge the scene
		virCam->updateModelViewProjMatrix(allGeometry[0]->dataRange.center, 1);
	else if (key == 's' || key == 'S')
		virCam->updateModelViewProjMatrix(allGeometry[0]->dataRange.center, 0);
		
	glutPostRedisplay();
}

int main(int argc, char** argv)
{	
	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(virtualCamWidth, virtualCamHeight); 
	glutInitWindowPosition (100, 100);
	glutCreateWindow ("Space Occupancy");	
	init();
	glutMouseFunc(mouse);
	glutMotionFunc(mouseTrack);
	glutSpecialFunc(inputKey);
	glutKeyboardFunc(keyboard);

	glutDisplayFunc(display); 	
	glutMainLoop();
	// clear the data
	/*for ( unsigned int i =0 ; i<allGeometry.size(); i++)
	{
		delete allGeometry[i];
	}
	for ( unsigned int i = 0; i<allCamView.size(); i++)
	{
		delete allCamView[i];
	}*/
	return 0;
}
	

