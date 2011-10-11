#include <gl\glew.h>

void init_glew_and_check_gl_version()
{	
	glewInit();
	GLint maxTexelUnits;
	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxTexelUnits);
	if (glewIsSupported("GL_VERSION_2_0"))
		printf("Ready for OpenGL 2.0\n");
	else {
		printf("OpenGL 2.0 not supported\n");
		exit(1);
	}
}

bool isInString(char *string, const char *search) {
	int pos=0;
	int maxpos=strlen(search)-1;
	int len=strlen(string);
	char *other;
	for (int i=0; i<len; i++) {
		if ((i==0) || ((i>1) && string[i-1]=='\n')) {			// New Extension Begins Here!
			other=&string[i];
			pos=0;							// Begin New Search
			while (string[i]!='\n') {				// Search Whole Extension-String
				if (string[i]==search[pos]) pos++;		// Next Position
				if ((pos>maxpos) && string[i+1]=='\n') return true;	// We Have A Winner!
				i++;
			}
		}
	}
	return false;								// Sorry, Not Found!
}


bool initMultitexture(void) 
{
	int maxTexelUnits;
	char *extensions;
	extensions=strdup((char *) glGetString(GL_EXTENSIONS));			// Fetch Extension String
	int len=strlen(extensions);
	for (int i=0; i<len; i++)						// Separate It By Newline Instead Of Blank
		if (extensions[i]==' ') extensions[i]='\n';

#ifdef EXT_INFO
	MessageBox(hWnd,extensions,"supported GL extensions",MB_OK | MB_ICONINFORMATION);
#endif

	if (isInString(extensions,"GL_ARB_multitexture")			// Is Multitexturing Supported?
		//&& __ARB_ENABLE							// Override Flag
		&& isInString(extensions,"GL_EXT_texture_env_combine"))		// texture-environment-combining supported?
	{       
		glGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxTexelUnits);
		glMultiTexCoord1fARB = (PFNGLMULTITEXCOORD1FARBPROC) wglGetProcAddress("glMultiTexCoord1fARB");
		glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC) wglGetProcAddress("glMultiTexCoord2fARB");
		glMultiTexCoord3fARB = (PFNGLMULTITEXCOORD3FARBPROC) wglGetProcAddress("glMultiTexCoord3fARB");
		glMultiTexCoord4fARB = (PFNGLMULTITEXCOORD4FARBPROC) wglGetProcAddress("glMultiTexCoord4fARB");
		glActiveTextureARB   = (PFNGLACTIVETEXTUREARBPROC) wglGetProcAddress("glActiveTextureARB");
		glClientActiveTextureARB= (PFNGLCLIENTACTIVETEXTUREARBPROC) wglGetProcAddress("glClientActiveTextureARB");
               
#ifdef EXT_INFO
		MessageBox(hWnd,"The GL_ARB_multitexture extension will be used.","feature supported!",MB_OK | MB_ICONINFORMATION);
#endif
		printf("=======Max Texture Unit: %d==========\n", maxTexelUnits);
		return true;
	}
	bool useMultitexture=false;							// We Can't Use It If It Isn't Supported!
	return false;
}