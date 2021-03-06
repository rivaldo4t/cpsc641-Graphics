#define _CRT_SECURE_NO_DEPRECATE
#include <GL/glew.h>
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GLint loc;
GLhandleARB shaderObjVert, shaderObjFrag, shaderProg;
float rotX = 0.0, rotY = 0.0;
float lpos[4] = { 1.0, 1.0, 1.0, 0.0 };

char *readFile(char *fn)
{
	FILE *fp;
	char *content = NULL;
	int count = 0;

	if (fn != NULL)
	{
		fp = fopen(fn, "rt");

		if (fp != NULL)
		{

			fseek(fp, 0, SEEK_END);
			count = ftell(fp);
			rewind(fp);

			if (count > 0)
			{
				content = (char *)malloc(sizeof(char) * (count + 1));
				count = fread(content, sizeof(char), count, fp);
				content[count] = '\0';
			}
			fclose(fp);
		}
	}
	return content;
}

void reshape(int w, int h) 
{
	if (h == 0)
		h = 1;
	float aspect = 1.0 * w / h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, w, h);

	gluPerspective(45, aspect, 1, 100);
	glMatrixMode(GL_MODELVIEW);
}

void render(void) 
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	gluLookAt(	0.0, 5.0, 5.0,
				0.0, 0.0, 0.0,
				0.0f, 1.0f, 0.0f);

	glLightfv(GL_LIGHT0, GL_POSITION, lpos);
	glRotatef(rotY, 0, 1, 0);
	glRotatef(rotX, 1, 0, 0);

	glutSolidTeapot(2);
	//glutSolidTorus(1, 2, 100, 100);
	//glutSolidIcosahedron();
	//glLineWidth(2);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glutSwapBuffers();
}

static void keyRot(int key, int x, int y)
{
	switch (key) 
	{
	case GLUT_KEY_UP:
		rotX -= 20.0;
		glutPostRedisplay();
		break;
	case GLUT_KEY_DOWN:
		rotX += 20.0;
		glutPostRedisplay();
		break;
	case GLUT_KEY_LEFT:
		rotY -= 20.0;
		glutPostRedisplay();
		break;
	case GLUT_KEY_RIGHT:
		rotY += 20.0;
		glutPostRedisplay();
		break;
	}
}

void quit(unsigned char key, int x, int y) 
{
	if (key == 27)
		exit(0);
}

#define printOpenGLError() printOglError(__FILE__, __LINE__)

int printOglError(char *file, int line)
{
	// Returns 1 if an OpenGL error occurred, 0 otherwise.
	GLenum glErr;
	int    retCode = 0;

	glErr = glGetError();
	while (glErr != GL_NO_ERROR)
	{
		printf("glError in file %s @ line %d: %s\n", file, line, gluErrorString(glErr));
		retCode = 1;
		glErr = glGetError();
	}
	return retCode;
}

void printInfoLog(GLhandleARB obj)
{
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog;

	glGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB,
		&infologLength);

	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetInfoLogARB(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n", infoLog);
		free(infoLog);
	}
}

void setShaders() 
{
	char *fileVert = NULL, *fileFrag = NULL;

	shaderObjVert = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	shaderObjFrag = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

	fileVert = readFile("npr.vert");
	fileFrag = readFile("npr.frag");

	const char * shaderVert = fileVert;
	const char * shaderFrag = fileFrag;

	glShaderSourceARB(shaderObjVert, 1, &shaderVert, NULL);
	glShaderSourceARB(shaderObjFrag, 1, &shaderFrag, NULL);

	free(fileVert);
	free(fileFrag);

	glCompileShaderARB(shaderObjVert);
	glCompileShaderARB(shaderObjFrag);

	printInfoLog(shaderObjVert);
	printInfoLog(shaderObjFrag);

	shaderProg = glCreateProgramObjectARB();
	glAttachObjectARB(shaderProg, shaderObjVert);
	glAttachObjectARB(shaderProg, shaderObjFrag);

	glLinkProgramARB(shaderProg);
	printInfoLog(shaderProg);

	glUseProgramObjectARB(shaderProg);

	loc = glGetUniformLocationARB(shaderProg, "time");
}

int main(int argc, char **argv) 
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(1500, 800);
	glutCreateWindow("NPR");

	glutDisplayFunc(render);
	glutIdleFunc(render);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(quit);
	glutSpecialFunc(keyRot);

	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	//glEnable(GL_CULL_FACE);

	glewInit();

	if (GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader)
		printf("GLSL supported\n");
	else 
	{
		printf("GLSL not support\n");
		exit(1);
	}

	setShaders();
	glutMainLoop();
	return 0;
}

