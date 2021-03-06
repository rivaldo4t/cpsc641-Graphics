#include "GL/glut.h"
#include <fstream>
#include "math.h"
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <stdlib.h>
using namespace std;

typedef struct { double x, y, z; } Vertex;
typedef Vertex Vector;
typedef struct { float r, g, b; } Color;

typedef struct
{
	int vertices[4]; // array of vertices (index into VertexArray)
	Color emissivity; // emitted radiosity
	Color reflectance; // reflectance
	Vertex center; // center of the patch
	Vector normal; // patch normal
	double area; // area of the patch
	Color radiosity; // radiosity of the patch
	Color unshot; // unshot radiosity of the patch
	int numelements; // number of elements in patch
	int startelement;  // number of first element for this patch in ElementArray
} Patch;

typedef struct
{
	int vertices[4];  // vertices of patch (index into VertexArray)
	Vertex center;  // center of the element
	double area;  // area of the element
	Patch* patch;  // Patch that this is an element of
	Color radiosity; // radiosity of the element
} Element;

int NumVertices;
Vertex* VertexArray;
Color* VertexColors;
int NumPatches;
Patch* PatchArray;
int NumElements;
Element* ElementArray;

inline double dot(Vector a, Vector b) { return a.x*b.x + a.y*b.y + a.z*b.z; }

// odd value to hemiCubeRes/2 causes glReadPixel to throw heap corruption error
float hemiCubeRes = 500;
double totalPatchArea;
Color R;
Color Initial_Ambient;
Color delta_Ambient;
vector<vector<double>> formFactors;
int numIter = 0;
#define WRITE_TO_CSV 1

void LoadFFData()
{
	formFactors = vector<vector<double>>(NumPatches, vector<double>(NumElements, 0.0));
#if WRITE_TO_CSV
	ifstream ip("FF.csv");

	//while (ip.good())
	//{
		int i, j;
		for (i = 0; i < NumPatches; i++)
		{
			for (j = 0; j < NumElements; j++)
			{
				string str;
				getline(ip, str, ',');
				// formFactors[i][j] = atof(&str[0]) / 0.00044158;
				formFactors[i][j] = atof(&str[0]) / 0.9;
			}
			//string str;
			//getline(ip, str, '\n');
		}
		//cout << formFactors[23][0] << endl;
	//}
#endif
}

void FormFactorCalculation()
{
	formFactors = vector<vector<double>>(NumPatches, vector<double>(NumElements, 0));
#if WRITE_TO_CSV
	ofstream outFF;
	outFF.open("FF.csv");
#endif
	for (int i = 0; i < NumPatches; i++)
	{
		Vector normal = PatchArray[i].normal;
		Vector planar_vector_1;
		Vector planar_vector_2;
		Vector centroid = PatchArray[i].center;

		planar_vector_1.x = VertexArray[PatchArray[i].vertices[0]].x - centroid.x;
		planar_vector_1.y = VertexArray[PatchArray[i].vertices[0]].y - centroid.y;
		planar_vector_1.z = VertexArray[PatchArray[i].vertices[0]].z - centroid.z;

		double mag = sqrt(planar_vector_1.x * planar_vector_1.x + planar_vector_1.y * planar_vector_1.y + planar_vector_1.z * planar_vector_1.z);

		planar_vector_1.x /= mag;
		planar_vector_1.y /= mag;
		planar_vector_1.z /= mag;

		planar_vector_2.x = normal.y * planar_vector_1.z - normal.z * planar_vector_1.y;
		planar_vector_2.y = normal.z * planar_vector_1.x - normal.x * planar_vector_1.z;
		planar_vector_2.z = normal.x * planar_vector_1.y - normal.y * planar_vector_1.x;

		float half_length = 1;
		float left, right, bottom, top;
		Vector eye = PatchArray[i].center;
		Vector lookat, up;
		Vector color;
		unsigned int viewWidth, viewHeight;
		int centerX, centerY;
		float PI_viewArea;
		float viewTemp = hemiCubeRes / (2 * half_length);
		float normX, normY;
			
		for (int k = 0; k < NumElements; k++)
		{
			if(k%1000 == 0)
				cout << "patch = " << i << " element = " << k << endl;

			for (int j = 1; j <= 5; j++)
			{
				switch (j)
				{
					case 1: left = -half_length;
							right = half_length;
							bottom = -half_length;
							top = half_length;
							lookat.x = centroid.x + normal.x;
							lookat.y = centroid.y + normal.y;
							lookat.z = centroid.z + normal.z;
							up = planar_vector_2;
							break;

					case 2:	left = -half_length;
							right = half_length;
							bottom = 0;
							top = half_length;
							lookat.x = centroid.x + planar_vector_1.x;
							lookat.y = centroid.y + planar_vector_1.y;
							lookat.z = centroid.z + planar_vector_1.z;
							up = normal;
							break;

					case 3:	left = -half_length;
							right = half_length;
							bottom = 0;
							top = half_length;
							lookat.x = centroid.x - planar_vector_1.x;
							lookat.y = centroid.y - planar_vector_1.y;
							lookat.z = centroid.z - planar_vector_1.z;
							up = normal;
							break;

					case 4:	left = -half_length;
							right = half_length;
							bottom = 0;
							top = half_length;
							lookat.x = centroid.x + planar_vector_2.x;
							lookat.y = centroid.y + planar_vector_2.y;
							lookat.z = centroid.z + planar_vector_2.z;
							up = normal;
							break;
					
					case 5:	left = -half_length;
							right = half_length;
							bottom = 0;
							top = half_length;
							lookat.x = centroid.x - planar_vector_2.x;
							lookat.y = centroid.y - planar_vector_2.y;
							lookat.z = centroid.z - planar_vector_2.z;
							up = normal;
							break;
				}
			
				viewWidth = (right - left) * viewTemp;
				viewHeight = (top - bottom) * viewTemp;
				centerX = viewWidth / 2;
				centerY = viewHeight / 2;
				normX = 1.0 / viewWidth;
				normY = 1.0 / viewHeight;
				PI_viewArea = viewWidth * viewHeight * 3.14159265;

				glViewport(0, 0, viewWidth, viewHeight);
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				glFrustum(left, right, bottom, top, half_length, 1000);
				gluLookAt(eye.x, eye.y, eye.z, lookat.x, lookat.y, lookat.z, up.x, up.y, up.z);
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

				for (int m = 0; m < NumElements; m++)
				{
					glBegin(GL_QUADS);

					if (m == k)
					{
						glColor3f(1.0f, 1.0f, 1.0f);
						glVertex3f(VertexArray[ElementArray[m].vertices[0]].x, VertexArray[ElementArray[m].vertices[0]].y, VertexArray[ElementArray[m].vertices[0]].z);
						glColor3f(1.0f, 1.0f, 1.0f);
						glVertex3f(VertexArray[ElementArray[m].vertices[1]].x, VertexArray[ElementArray[m].vertices[1]].y, VertexArray[ElementArray[m].vertices[1]].z);
						glColor3f(1.0f, 1.0f, 1.0f);
						glVertex3f(VertexArray[ElementArray[m].vertices[2]].x, VertexArray[ElementArray[m].vertices[2]].y, VertexArray[ElementArray[m].vertices[2]].z);
						glColor3f(1.0f, 1.0f, 1.0f);
						glVertex3f(VertexArray[ElementArray[m].vertices[3]].x, VertexArray[ElementArray[m].vertices[3]].y, VertexArray[ElementArray[m].vertices[3]].z);
					}
					else
					{
						glColor3f(0.0f, 0.0f, 0.0f);
						glVertex3f(VertexArray[ElementArray[m].vertices[0]].x, VertexArray[ElementArray[m].vertices[0]].y, VertexArray[ElementArray[m].vertices[0]].z);
						glColor3f(0.0f, 0.0f, 0.0f);
						glVertex3f(VertexArray[ElementArray[m].vertices[1]].x, VertexArray[ElementArray[m].vertices[1]].y, VertexArray[ElementArray[m].vertices[1]].z);
						glColor3f(0.0f, 0.0f, 0.0f);
						glVertex3f(VertexArray[ElementArray[m].vertices[2]].x, VertexArray[ElementArray[m].vertices[2]].y, VertexArray[ElementArray[m].vertices[2]].z);
						glColor3f(0.0f, 0.0f, 0.0f);
						glVertex3f(VertexArray[ElementArray[m].vertices[3]].x, VertexArray[ElementArray[m].vertices[3]].y, VertexArray[ElementArray[m].vertices[3]].z);
					}

					glEnd();
					//glFlush();
				}

				GLubyte* buffer = new GLubyte[viewWidth * viewHeight * 3];
				glReadPixels(0, 0, viewWidth, viewHeight, GL_RGB, GL_UNSIGNED_BYTE, buffer);
				// cout << "GL_ERROR : " << glGetError() << endl;

				for (int y = 0; y < viewHeight; y++)
				{
					for (int x = 0; x < viewWidth; x++)
					{
						color.x = buffer[(y*viewWidth + x) * 3];
						color.y = buffer[(y*viewWidth + x) * 3 + 1];
						color.z = buffer[(y*viewWidth + x) * 3 + 2];

						if (color.x < 1.0 && color.y < 1.0 && color.z < 1.0)
							continue;
						else
						{
							if (j == 1)
								// formFactors[i][k] += 1.0 / (PI_pixelArea * (pow(pow(centerX - x, 2) + pow(centerY - y, 2) + pow(half_length, 2), 2)));
								formFactors[i][k] += 1.0 / (PI_viewArea * (pow(pow(normX * (centerX - x), 2) + pow(normY * (centerY - y), 2) + 1, 2)));
							else
								// formFactors[i][k] += y / (PI_pixelArea * (pow(pow(centerX - x, 2) + pow(y, 2) + pow(half_length, 2), 2)));
								formFactors[i][k] += (normY * y) / (PI_viewArea * (pow(pow(normX * (centerX - x), 2) + pow(normY * y, 2) + 1, 2)));
						}
					}
				}

				delete buffer;
			}
#if WRITE_TO_CSV		
			outFF << formFactors[i][k] << ",";
#endif
		}
#if WRITE_TO_CSV
		outFF << endl;
#endif
		cout << i << " " << formFactors[i][2500] << endl;
	}
#if WRITE_TO_CSV
	outFF.close();
#endif
}

void RadiosityCalculation()
{
	for (int i = 0; i < NumPatches; i++)
	{
		//cout << formFactors[i][2000] << endl;
		/*cout << delta_Ambient.r << endl << delta_Ambient.g << endl << delta_Ambient.b << endl;
		cout << "---------------------\n";*/
		for (int j = 0; j < NumElements; j++)
		{
			Color delta_Rad;
			Patch* element_patch = ElementArray[j].patch;
			
			/*if(PatchArray[i].unshot.r > 0 || PatchArray[i].unshot.g > 0 || PatchArray[i].unshot.b > 0)
				if(formFactors[i][j] > 0)
					int a = 0;*/

			delta_Rad.r = element_patch->reflectance.r * PatchArray[i].unshot.r * formFactors[i][j] * PatchArray[i].area / ElementArray[j].area;
			delta_Rad.g = element_patch->reflectance.g * PatchArray[i].unshot.g * formFactors[i][j] * PatchArray[i].area / ElementArray[j].area;
			delta_Rad.b = element_patch->reflectance.b * PatchArray[i].unshot.b * formFactors[i][j] * PatchArray[i].area / ElementArray[j].area;

			ElementArray[j].radiosity.r += delta_Rad.r + (element_patch->reflectance.r * delta_Ambient.r);
			ElementArray[j].radiosity.g += delta_Rad.g + (element_patch->reflectance.g * delta_Ambient.g);
			ElementArray[j].radiosity.b += delta_Rad.b + (element_patch->reflectance.b * delta_Ambient.b);

			element_patch->unshot.r += delta_Rad.r * ElementArray[j].area / element_patch->area;
			element_patch->unshot.g += delta_Rad.g * ElementArray[j].area / element_patch->area;
			element_patch->unshot.b += delta_Rad.b * ElementArray[j].area / element_patch->area;
		}

		PatchArray[i].unshot.r = 0;
		PatchArray[i].unshot.g = 0;
		PatchArray[i].unshot.b = 0;

		Color delta_BA = {};
		for (int n = 0; n < NumPatches; n++)
		{
			delta_BA.r += PatchArray[n].unshot.r * PatchArray[n].area;
			delta_BA.g += PatchArray[n].unshot.g * PatchArray[n].area;
			delta_BA.b += PatchArray[n].unshot.b * PatchArray[n].area;
		}

		delta_Ambient.r = R.r * delta_BA.r / totalPatchArea;
		delta_Ambient.g = R.g * delta_BA.g / totalPatchArea;
		delta_Ambient.b = R.b * delta_BA.b / totalPatchArea;
		//cout << delta_BA.r << endl << delta_BA.g << endl << delta_BA.b << endl;
		//cout << "---------------------\n";
	}

	for (int m = 0; m < NumElements; m++)
	{
		VertexColors[ElementArray[m].vertices[0]].r += ElementArray[m].radiosity.r;
		VertexColors[ElementArray[m].vertices[0]].r /= 2;
		VertexColors[ElementArray[m].vertices[0]].g += ElementArray[m].radiosity.g;
		VertexColors[ElementArray[m].vertices[0]].g /= 2;
		VertexColors[ElementArray[m].vertices[0]].b += ElementArray[m].radiosity.b;
		VertexColors[ElementArray[m].vertices[0]].b /= 2;
		VertexColors[ElementArray[m].vertices[1]].r += ElementArray[m].radiosity.r;
		VertexColors[ElementArray[m].vertices[1]].r /= 2;
		VertexColors[ElementArray[m].vertices[1]].g += ElementArray[m].radiosity.g;
		VertexColors[ElementArray[m].vertices[1]].g /= 2;
		VertexColors[ElementArray[m].vertices[1]].b += ElementArray[m].radiosity.b;
		VertexColors[ElementArray[m].vertices[1]].b /= 2;
		VertexColors[ElementArray[m].vertices[2]].r += ElementArray[m].radiosity.r;
		VertexColors[ElementArray[m].vertices[2]].r /= 2;
		VertexColors[ElementArray[m].vertices[2]].g += ElementArray[m].radiosity.g;
		VertexColors[ElementArray[m].vertices[2]].g /= 2;
		VertexColors[ElementArray[m].vertices[2]].b += ElementArray[m].radiosity.b;
		VertexColors[ElementArray[m].vertices[2]].b /= 2;
		VertexColors[ElementArray[m].vertices[3]].r += ElementArray[m].radiosity.r;
		VertexColors[ElementArray[m].vertices[3]].r /= 2;
		VertexColors[ElementArray[m].vertices[3]].g += ElementArray[m].radiosity.g;
		VertexColors[ElementArray[m].vertices[3]].g /= 2;
		VertexColors[ElementArray[m].vertices[3]].b += ElementArray[m].radiosity.b;
		VertexColors[ElementArray[m].vertices[3]].b /= 2;
	}
}

void DoStep(void)
{
	// Do one step of Progressive Refinement
	cout << "Iteration # " << numIter++ << endl;

	RadiosityCalculation();
}

int LoadData(void)
{
	int i, j, k;
	int nverts, vertnum, startvert;
	int elnum;
	Vertex* vtemp;
	ifstream infi("Test.dat");
	Vector v1;
	Vector v2;
	double length1, length2;
	double temparea;

	NumElements = 0;

	// read initial vertices
	infi >> nverts;
	vtemp = new Vertex[nverts];
	for (i = 0;i<nverts;i++)
	{
		infi >> vtemp[i].x >> vtemp[i].y >> vtemp[i].z;
	}
	NumVertices = nverts;

	// read patches
	infi >> NumPatches;
	PatchArray = new Patch[NumPatches];

	totalPatchArea = 0;
	Color area_weighted_reflectivity = {};
	Color avg_reflectivity = {};
	R = {};
	Initial_Ambient = {};
	delta_Ambient = {};
	Color EA = {};
	//Color sum = {};
	for (i = 0; i < NumPatches; i++)
	{
		// Read patch i
		// A patch is represented by quad
		infi >> PatchArray[i].vertices[0] >> PatchArray[i].vertices[1] >> PatchArray[i].vertices[2] >> PatchArray[i].vertices[3];
		infi >> PatchArray[i].emissivity.r >> PatchArray[i].emissivity.g >> PatchArray[i].emissivity.b;
		infi >> PatchArray[i].reflectance.r >> PatchArray[i].reflectance.g >> PatchArray[i].reflectance.b;
		infi >> PatchArray[i].numelements;
		
		/*if (PatchArray[i].emissivity.r > 0)
		{
			cout << PatchArray[i].emissivity.r << endl << PatchArray[i].emissivity.g << endl << PatchArray[i].emissivity.b << endl;
			cout << "---------------------\n";
		}*/
		/*if (PatchArray[i].reflectance.r > 0)
		{
		cout << PatchArray[i].reflectance.r << endl << PatchArray[i].reflectance.g << endl << PatchArray[i].reflectance.b << endl;
		cout << "---------------------\n";
		}*/

		// ??
		NumVertices += (PatchArray[i].numelements + 1) * (PatchArray[i].numelements + 1);
		PatchArray[i].startelement = NumElements;
		NumElements += (PatchArray[i].numelements * PatchArray[i].numelements);

		// Centre of the quad
		PatchArray[i].center.x = (vtemp[PatchArray[i].vertices[0]].x + vtemp[PatchArray[i].vertices[1]].x + vtemp[PatchArray[i].vertices[2]].x + vtemp[PatchArray[i].vertices[3]].x) / 4.0;
		PatchArray[i].center.y = (vtemp[PatchArray[i].vertices[0]].y + vtemp[PatchArray[i].vertices[1]].y + vtemp[PatchArray[i].vertices[2]].y + vtemp[PatchArray[i].vertices[3]].y) / 4.0;
		PatchArray[i].center.z = (vtemp[PatchArray[i].vertices[0]].z + vtemp[PatchArray[i].vertices[1]].z + vtemp[PatchArray[i].vertices[2]].z + vtemp[PatchArray[i].vertices[3]].z) / 4.0;

		// Vectors of two sides of quad
		v1.x = vtemp[PatchArray[i].vertices[1]].x - vtemp[PatchArray[i].vertices[0]].x;
		v1.y = vtemp[PatchArray[i].vertices[1]].y - vtemp[PatchArray[i].vertices[0]].y;
		v1.z = vtemp[PatchArray[i].vertices[1]].z - vtemp[PatchArray[i].vertices[0]].z;
		v2.x = vtemp[PatchArray[i].vertices[3]].x - vtemp[PatchArray[i].vertices[0]].x;
		v2.y = vtemp[PatchArray[i].vertices[3]].y - vtemp[PatchArray[i].vertices[0]].y;
		v2.z = vtemp[PatchArray[i].vertices[3]].z - vtemp[PatchArray[i].vertices[0]].z;

		length1 = sqrt(v1.x*v1.x + v1.y*v1.y + v1.z*v1.z);
		length2 = sqrt(v2.x*v2.x + v2.y*v2.y + v2.z*v2.z);

		// area of patch
		PatchArray[i].area = length1*length2;

		// normalize vectors
		v1.x /= length1;
		v1.y /= length1;
		v1.z /= length1;
		v2.x /= length2;
		v2.y /= length2;
		v2.z /= length2;

		// Cross the quad-side vectors to get normal
		PatchArray[i].normal.x = v1.y*v2.z - v1.z*v2.y;
		PatchArray[i].normal.y = v2.x*v1.z - v1.x*v2.z;
		PatchArray[i].normal.z = v1.x*v2.y - v1.y*v2.x;

		// Initially, radiosity is equal to the emissivity, i.e, only light sources have initial radiosity
		PatchArray[i].radiosity.r = PatchArray[i].emissivity.r;
		PatchArray[i].radiosity.g = PatchArray[i].emissivity.g;
		PatchArray[i].radiosity.b = PatchArray[i].emissivity.b;

		// Initially, all the emissivity is unshot
		PatchArray[i].unshot.r = PatchArray[i].emissivity.r;
		PatchArray[i].unshot.g = PatchArray[i].emissivity.g;
		PatchArray[i].unshot.b = PatchArray[i].emissivity.b;

		// Total Patch area
		totalPatchArea += PatchArray[i].area;

		// Summation of p_i * A_i
		area_weighted_reflectivity.r += PatchArray[i].reflectance.r * PatchArray[i].area;
		area_weighted_reflectivity.g += PatchArray[i].reflectance.g * PatchArray[i].area;
		area_weighted_reflectivity.b += PatchArray[i].reflectance.b * PatchArray[i].area;

		// Area Weigted emmisivity for initial ambient term
		EA.r += PatchArray[i].emissivity.r * PatchArray[i].area;
		EA.g += PatchArray[i].emissivity.g * PatchArray[i].area;
		EA.b += PatchArray[i].emissivity.b * PatchArray[i].area;

		/*sum.r += PatchArray[i].reflectance.r;
		sum.g += PatchArray[i].reflectance.r;
		sum.b += PatchArray[i].reflectance.r;*/
	}
	//cout << sum.r / 150 << endl << sum.g / 150 << endl << sum.b / 150 << endl;

	// Determine Reflection factor, R
	avg_reflectivity.r = area_weighted_reflectivity.r / totalPatchArea;
	avg_reflectivity.g = area_weighted_reflectivity.g / totalPatchArea;
	avg_reflectivity.b = area_weighted_reflectivity.b / totalPatchArea;

	R.r = 1.0 / (1 - avg_reflectivity.r);
	R.g = 1.0 / (1 - avg_reflectivity.g);
	R.b = 1.0 / (1 - avg_reflectivity.b);

	double Rmag = sqrt(R.r*R.r + R.g*R.g + R.b*R.b);
	R.r /= Rmag * 150;
	R.g /= Rmag * 150;
	R.b /= Rmag * 150;
	
	/*R.r = 0.004;
	R.g = 0.004;
	R.b = 0.004;*/

	// Initial Ambient term
	Initial_Ambient.r = R.r * EA.r / totalPatchArea;
	Initial_Ambient.g = R.g * EA.g / totalPatchArea;
	Initial_Ambient.b = R.b * EA.b / totalPatchArea;

	/*Initial_Ambient.r = 0.0;
	Initial_Ambient.g = 0.0;
	Initial_Ambient.b = 0.0;*/

	// create elements (including new vertices)
	VertexArray = new Vertex[NumVertices];
	VertexColors = new Color[NumVertices];
	ElementArray = new Element[NumElements];

	// Copy original (patch) vertices to beginning of array
	for (i = 0;i<nverts;i++)
	{
		VertexArray[i].x = vtemp[i].x;
		VertexArray[i].y = vtemp[i].y;
		VertexArray[i].z = vtemp[i].z;

		VertexColors[i].r = 0.0;
		VertexColors[i].g = 0.0;
		VertexColors[i].b = 0.0;
	}

	// Form Vertices for new elements
	vertnum = nverts;
	elnum = 0;
	for (i = 0;i<NumPatches;i++)
	{
		v1.x = (VertexArray[PatchArray[i].vertices[1]].x - VertexArray[PatchArray[i].vertices[0]].x) / PatchArray[i].numelements;
		v1.y = (VertexArray[PatchArray[i].vertices[1]].y - VertexArray[PatchArray[i].vertices[0]].y) / PatchArray[i].numelements;
		v1.z = (VertexArray[PatchArray[i].vertices[1]].z - VertexArray[PatchArray[i].vertices[0]].z) / PatchArray[i].numelements;
		v2.x = (VertexArray[PatchArray[i].vertices[3]].x - VertexArray[PatchArray[i].vertices[0]].x) / PatchArray[i].numelements;
		v2.y = (VertexArray[PatchArray[i].vertices[3]].y - VertexArray[PatchArray[i].vertices[0]].y) / PatchArray[i].numelements;
		v2.z = (VertexArray[PatchArray[i].vertices[3]].z - VertexArray[PatchArray[i].vertices[0]].z) / PatchArray[i].numelements;

		startvert = vertnum;

		for (j = 0;j<PatchArray[i].numelements + 1;j++)
		{
			for (k = 0;k<PatchArray[i].numelements + 1;k++)
			{
				// Create new vertex
				VertexArray[vertnum].x = VertexArray[PatchArray[i].vertices[0]].x + k * v1.x + j * v2.x;
				VertexArray[vertnum].y = VertexArray[PatchArray[i].vertices[0]].y + k * v1.y + j * v2.y;
				VertexArray[vertnum].z = VertexArray[PatchArray[i].vertices[0]].z + k * v1.z + j * v2.z;

				VertexColors[vertnum].r = 0.0;
				VertexColors[vertnum].g = 0.0;
				VertexColors[vertnum].b = 0.0;

				vertnum++;
			}
		}
		temparea = PatchArray[i].area / (PatchArray[i].numelements * PatchArray[i].numelements);
		
		// Form Elements for new patch
		for (j = 0; j < PatchArray[i].numelements; j++)
		{
			for (k = 0 ; k < PatchArray[i].numelements; k++)
			{
				// Set vertices
				ElementArray[elnum].vertices[0] = startvert + k + j * (PatchArray[i].numelements + 1);
				ElementArray[elnum].vertices[1] = startvert + (k + 1) + j * (PatchArray[i].numelements + 1);
				ElementArray[elnum].vertices[2] = startvert + (k + 1) + (j + 1) * (PatchArray[i].numelements + 1);
				ElementArray[elnum].vertices[3] = startvert + k + (j + 1) * (PatchArray[i].numelements + 1);

				// Set center
				ElementArray[elnum].center.x =
					(VertexArray[ElementArray[elnum].vertices[0]].x +
						VertexArray[ElementArray[elnum].vertices[1]].x +
						VertexArray[ElementArray[elnum].vertices[2]].x +
						VertexArray[ElementArray[elnum].vertices[3]].x) / 4.0;
				ElementArray[elnum].center.y =
					(VertexArray[ElementArray[elnum].vertices[0]].y +
						VertexArray[ElementArray[elnum].vertices[1]].y +
						VertexArray[ElementArray[elnum].vertices[2]].y +
						VertexArray[ElementArray[elnum].vertices[3]].y) / 4.0;
				ElementArray[elnum].center.z =
					(VertexArray[ElementArray[elnum].vertices[0]].z +
						VertexArray[ElementArray[elnum].vertices[1]].z +
						VertexArray[ElementArray[elnum].vertices[2]].z +
						VertexArray[ElementArray[elnum].vertices[3]].z) / 4.0;

				ElementArray[elnum].area = temparea;
				ElementArray[elnum].patch = &PatchArray[i];
				
				ElementArray[elnum].radiosity.r = (PatchArray[i].reflectance.r * Initial_Ambient.r) + PatchArray[i].emissivity.r;
				ElementArray[elnum].radiosity.g = (PatchArray[i].reflectance.g * Initial_Ambient.g) + PatchArray[i].emissivity.g;
				ElementArray[elnum].radiosity.b = (PatchArray[i].reflectance.b * Initial_Ambient.b) + PatchArray[i].emissivity.b;
#if 1
				VertexColors[ElementArray[elnum].vertices[0]].r = ElementArray[elnum].radiosity.r;
				VertexColors[ElementArray[elnum].vertices[0]].g = ElementArray[elnum].radiosity.g;
				VertexColors[ElementArray[elnum].vertices[0]].b = ElementArray[elnum].radiosity.b;
				VertexColors[ElementArray[elnum].vertices[1]].r = ElementArray[elnum].radiosity.r;
				VertexColors[ElementArray[elnum].vertices[1]].g = ElementArray[elnum].radiosity.g;
				VertexColors[ElementArray[elnum].vertices[1]].b = ElementArray[elnum].radiosity.b;
				VertexColors[ElementArray[elnum].vertices[2]].r = ElementArray[elnum].radiosity.r;
				VertexColors[ElementArray[elnum].vertices[2]].g = ElementArray[elnum].radiosity.g;
				VertexColors[ElementArray[elnum].vertices[2]].b = ElementArray[elnum].radiosity.b;
				VertexColors[ElementArray[elnum].vertices[3]].r = ElementArray[elnum].radiosity.r;
				VertexColors[ElementArray[elnum].vertices[3]].g = ElementArray[elnum].radiosity.g;
				VertexColors[ElementArray[elnum].vertices[3]].b = ElementArray[elnum].radiosity.b;
#endif
				elnum++;
			}
		}
	}

	delete[] vtemp;
	infi.close();
	return 0;
}

void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-5.0, 5.0, -4.0, 4.0, 10.1, 25.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(20.0, 5.0, 4.0, 10.0, 5.0, 4.0, 0.0, 0.0, 1.0);
}

void display(void)
{
	int i;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBegin(GL_QUADS);
	for (i = 0;i<NumElements;i++)
	{
		glColor3f(VertexColors[ElementArray[i].vertices[0]].r,
			VertexColors[ElementArray[i].vertices[0]].g,
			VertexColors[ElementArray[i].vertices[0]].b);
		glVertex3f(VertexArray[ElementArray[i].vertices[0]].x,
			VertexArray[ElementArray[i].vertices[0]].y,
			VertexArray[ElementArray[i].vertices[0]].z);
		glColor3f(VertexColors[ElementArray[i].vertices[1]].r,
			VertexColors[ElementArray[i].vertices[1]].g,
			VertexColors[ElementArray[i].vertices[1]].b);
		glVertex3f(VertexArray[ElementArray[i].vertices[1]].x,
			VertexArray[ElementArray[i].vertices[1]].y,
			VertexArray[ElementArray[i].vertices[1]].z);
		glColor3f(VertexColors[ElementArray[i].vertices[2]].r,
			VertexColors[ElementArray[i].vertices[2]].g,
			VertexColors[ElementArray[i].vertices[2]].b);
		glVertex3f(VertexArray[ElementArray[i].vertices[2]].x,
			VertexArray[ElementArray[i].vertices[2]].y,
			VertexArray[ElementArray[i].vertices[2]].z);
		glColor3f(VertexColors[ElementArray[i].vertices[3]].r,
			VertexColors[ElementArray[i].vertices[3]].g,
			VertexColors[ElementArray[i].vertices[3]].b);
		glVertex3f(VertexArray[ElementArray[i].vertices[3]].x,
			VertexArray[ElementArray[i].vertices[3]].y,
			VertexArray[ElementArray[i].vertices[3]].z);
	}
	glEnd();
	glFlush();
}

void init(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glEnable(GL_DEPTH_TEST);
	LoadData();
	//FormFactorCalculation();
	LoadFFData();
	cout << "Data Successfully Loaded\n";
	reshape(800, 640);
}

void mouse(int button, int state, int x, int y)
{
	static int pressed = 0;

	if (button == GLUT_LEFT_BUTTON)
	{
		if ((state == GLUT_DOWN) && (pressed == 0))
		{
			//Enable below line to see number of iterations done on a single click
			for(int i = 0; i < 500; i++)
				DoStep();
			glutPostRedisplay();
			pressed = 1;
		}
		else if ((state == GLUT_UP) && (pressed == 1))
		{
			pressed = 0;
		}
	}
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 640);
	glutInitWindowPosition(400, 100);
	glutCreateWindow("Tushar - Assignment 3");
	init();
	glutDisplayFunc(display);
	glutMouseFunc(mouse);
	glutReshapeFunc(reshape);
	glutMainLoop();
	return 0;
}
