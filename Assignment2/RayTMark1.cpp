// RayTMark1.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <algorithm>
#include <GL/glut.h>
#include <fstream>
#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;

#define ImageW 600
#define ImageH 400

float framebuffer[ImageH][ImageW][3];

struct Coord3D { double x, y, z; };
struct Material
{
	Coord3D color;
	float ambient_factor;
	float diffuse_factor;
	float specular_factor;
	float roughness;
};

Coord3D Light;
Coord3D SphCent[6];
double SphRad[6];
Material SphProp[6];

double cu[36][3] = 
{
	{ 370.0, 1.36, 1.975 },
	{ 380.0, 1.33, 2.045 },
	{ 390.0, 1.33, 2.045 },
	{ 400.0, 1.32, 2.116 },
	{ 410.0, 1.28, 2.207 },
	{ 420.0, 1.28, 2.207 },
	{ 430.0, 1.25, 2.305 },
	{ 440.0, 1.25, 2.305 },
	{ 450.0, 1.24, 2.397 },
	{ 460.0, 1.24, 2.397 },
	{ 470.0, 1.25, 2.483 },
	{ 480.0, 1.25, 2.483 },
	{ 490.0, 1.22, 2.564 },
	{ 500.0, 1.22, 2.564 },
	{ 510.0, 1.18, 2.608 },
	{ 520.0, 1.18, 2.608 },
	{ 530.0, 1.18, 2.608 },
	{ 540.0, 1.02, 2.577 },
	{ 550.0, 1.02, 2.577 },
	{ 560.0, 1.02, 2.577 },
	{ 570.0, 0.7, 2.704	 },
	{ 580.0, 0.7, 2.704	 },
	{ 590.0, 0.7, 2.704	 },
	{ 600.0, 0.7, 2.704	 },
	{ 610.0, 0.3, 3.205	 },
	{ 620.0, 0.3, 3.205	 },
	{ 630.0, 0.3, 3.205	 },
	{ 640.0, 0.3, 3.205	 },
	{ 650.0, 0.22, 3.747 },
	{ 660.0, 0.22, 3.747 },
	{ 670.0, 0.22, 3.747 },
	{ 680.0, 0.22, 3.747 },
	{ 690.0, 0.21, 4.205 },
	{ 700.0, 0.21, 4.205 },
	{ 710.0, 0.21, 4.205 },
	{ 720.0, 0.21, 4.205 }
};

double dataTable[36][12] = 
{
	// 0:			Wavelength(nm)
	// 1:			Solar Irradiance Global Tilt(W*m-2*nm-1)
	// 2,3:			Refractive index, Extinction coeff - Gold
	// 4,5:			Refractive index, Extinction coeff - Aluminium
	// 6,7:			Refractive index, Extinction coeff - Amorphous Silicon
	// 8:			Refractive index - Quartz
	// 9, 10, 11:	x, y, z

	{ 370.0, 0.75507, 1.48, 1.895, 0.39877, 4.3957, 3.9000, 2.6600, 1.474468282, 0.0004149, 0.00001239, 0.001946 },
	{ 380.0, 0.70077, 1.46, 1.933, 0.39877, 4.3957, 3.9000, 2.6600, 1.472704680, 0.001368, 0.000039, 0.006450001 },
	{ 390.0, 0.79699, 1.46, 1.933, 0.39877, 4.3957, 4.1700, 2.3800, 1.471052512, 0.004243, 0.00012, 0.02005001 },
	{ 400.0, 1.11410, 1.47, 1.952, 0.52135, 5.0008, 4.1700, 2.3800, 1.471052512, 0.01431, 0.000396, 0.06785001 },
	{ 410.0, 1.04850, 1.46, 1.958, 0.52135, 5.0008, 4.3800, 2.0200, 1.46952865,  0.04351, 0.00121, 0.2074 },
	{ 420.0, 1.12320, 1.46, 1.958, 0.52135, 5.0008, 4.3800, 2.0200, 1.468121822, 0.13438, 0.004, 0.6456 },
	{ 430.0, 0.87462, 1.45, 1.948, 0.52135, 5.0008, 4.3800, 2.0200, 1.468121822, 0.2839, 0.0116, 1.3856 },
	{ 440.0, 1.34990, 1.45, 1.948, 0.6079,  5.3676, 4.4700, 1.6400, 1.46680482,  0.34828, 0.023, 1.74706 },
	{ 450.0, 1.55950, 1.38, 1.914, 0.6079,  5.3676, 4.4700, 1.6400, 1.46558083,  0.3362, 0.038, 1.77211 },
	{ 460.0, 1.52910, 1.38, 1.914, 0.6079,  5.3676, 4.4700, 1.6400, 1.46558083,  0.2908, 0.06, 1.6692 },
	{ 470.0, 1.50770, 1.31, 1.849, 0.6079,  5.3676, 4.4900, 1.2800, 1.464436031, 0.19536, 0.09098, 1.28764 },
	{ 480.0, 1.61810, 1.31, 1.849, 0.7278,  5.7781, 4.4900, 1.2800, 1.463371935, 0.09564, 0.13902, 0.8129501 },
	{ 490.0, 1.62240, 1.04, 1.833, 0.7278,  5.7781, 4.4900, 1.2800, 1.462376439, 0.03201, 0.20802, 0.46518 },
	{ 500.0, 1.54510, 1.04, 1.833, 0.7278,  5.7781, 4.4700, 1.1200, 1.462376439, 0.0049, 0.323, 0.272 },
	{ 510.0, 1.54810, 1.04, 1.833, 0.7278,  5.7781, 4.4700, 1.1200, 1.462376439, 0.0093, 0.503, 0.1582 },
	{ 520.0, 1.52360, 0.62, 2.081, 0.8734,  6.2418, 4.4600, 0.9690, 1.461444991, 0.06327, 0.71, 0.07824999 },
	{ 530.0, 1.54460, 0.62, 2.081, 0.8734,  6.2418, 4.4600, 0.9690, 1.461444991, 0.1655, 0.862, 0.04216 },
	{ 540.0, 1.48250, 0.43, 2.455, 0.8734,  6.2418, 4.4600, 0.9690, 1.460573079, 0.2904, 0.954, 0.0203 },
	{ 550.0, 1.53990, 0.43, 2.455, 0.8734,  6.2418, 4.4600, 0.9690, 1.459756285, 0.4334499, 0.9949501, 0.008749999 },
	{ 560.0, 1.47400, 0.43, 2.455, 1.0728,  6.7839, 4.3600, 0.6900, 1.459756285, 0.5945, 0.995, 0.0039 },
	{ 570.0, 1.48160, 0.29, 2.863, 1.0728,  6.7839, 4.3600, 0.6900, 1.458986561, 0.7621, 0.952, 0.0021 },
	{ 580.0, 1.50200, 0.29, 2.863, 1.0728,  6.7839, 4.3600, 0.6900, 1.458986561, 0.9163, 0.870, 0.001650001 },
	{ 590.0, 1.37090, 0.29, 2.863, 1.0728,  6.7839, 4.3600, 0.6900, 1.458260788, 1.0263, 0.757, 0.0011 },
	{ 600.0, 1.47530, 0.21, 3.272, 1.366,   7.4052, 4.2300, 0.4610, 1.458260788, 1.0622, 0.631, 0.0008 },
	{ 610.0, 1.46860, 0.21, 3.272, 1.366,   7.4052, 4.2300, 0.4610, 1.457575807, 1.0026, 0.503, 0.00034 },
	{ 620.0, 1.47390, 0.21, 3.272, 1.366,   7.4052, 4.2300, 0.4610, 1.457575807, 0.8544499, 0.381, 0.00019 },
	{ 630.0, 1.39240, 0.21, 3.272, 1.366,   7.4052, 4.2300, 0.4610, 1.457575807, 0.6424, 0.265, 0.00004999999 },
	{ 640.0, 1.43400, 0.14, 3.697, 1.366,   7.4052, 4.1700, 0.3630, 1.456925601, 0.4479, 0.175, 0.00002 },
	{ 650.0, 1.35940, 0.14, 3.697, 1.5724,  7.7354, 4.1700, 0.3630, 1.456925601, 0.2835, 0.107, 0 },
	{ 660.0, 1.39920, 0.14, 3.697, 1.5724,  7.7354, 4.1700, 0.3630, 1.456310408, 0.1649, 0.061, 0 },
	{ 670.0, 1.41960, 0.14, 3.697, 1.5724,  7.7354, 4.1700, 0.3630, 1.456310408, 0.0874, 0.032, 0 },
	{ 680.0, 1.39690, 0.14, 3.697, 1.8301,  8.0601, 4.0900, 0.2710, 1.455724699, 0.04677, 0.017, 0 },
	{ 690.0, 1.18210, 0.13, 4.103, 1.8301,  8.0601, 4.0900, 0.2710, 1.455724699, 0.0227, 0.00821, 0 },
	{ 700.0, 1.28230, 0.13, 4.103, 1.8301,  8.0601, 4.0900, 0.2710, 1.455724699, 0.01135916, 0.004102, 0 },
	{ 710.0, 1.31750, 0.13, 4.103, 2.1606,  8.3565, 4.0100, 0.1990, 1.45516603,  0.005790346, 0.002091, 0 },
	{ 720.0, 0.98550, 0.13, 4.103, 2.1606,  8.3565, 4.0100, 0.1990, 1.454629875, 0.002899327, 0.001047, 0 }
};

// Normalizes the vector passed in
void normalize(double& x, double& y, double& z) {
	float temp = sqrt(x*x + y*y + z*z);
	if (temp > 0.0) {
		x /= temp;
		y /= temp;
		z /= temp;
	}
	else {
		x = 0.0;
		y = 0.0;
		z = 0.0;
	}
}

// Returns dot product of two vectors
float dot(double x1, double y1, double z1, double x2, double y2, double z2) {
	return (x1*x2 + y1*y2 + z1*z2);
}

// Returns angle between two vectors
float angle(double x1, double y1, double z1, double x2, double y2, double z2) {
	normalize(x1, y1, z1);
	normalize(x2, y2, z2);
	return  acos(dot(x1, y1, z1, x2, y2, z2));
}

//************* THIS IS THE ROUTINE YOU WOULD NEED TO WRITE *****************
// Get Color for a point on the surface
void GetColor(Coord3D view,   // Normalized Vector pointing FROM eye TO surface
	Coord3D normal, // Normalized Vector giving surface normal
	Coord3D light,  // Normalized Vector pointing FROM surface TO light
	int SphNum,     // Sphere Number (0-5)
	float& R,       // Return these values for surface color.
	float& G,
	float& B) 
{
	// This line is just here so you can see where the spheres are to begin with.
	//R = dot(light.x, light.y, light.z, normal.x, normal.y, normal.z)*(SphProp[SphNum].color.x);
	//G = dot(light.x, light.y, light.z, normal.x, normal.y, normal.z)*(SphProp[SphNum].color.y);
	//B = dot(light.x, light.y, light.z, normal.x, normal.y, normal.z)*(SphProp[SphNum].color.z);

	Coord3D v = { -(view.x), -(view.y), -(view.z) };

	double r = 0;
	double g = 0;
	double b = 0;
	Coord3D XYZ = { 0.0f, 0.0f, 0.0f };

	//Get data from table and set material properties
	for (int wavelengthIndex = 0; wavelengthIndex < 36; wavelengthIndex += 1)
	{
		//int wavelengthIndex = 5;
		double waveLength = dataTable[wavelengthIndex][0];
		double irradiance = dataTable[wavelengthIndex][1];
		double lambda_x = dataTable[wavelengthIndex][9];
		double lambda_y = dataTable[wavelengthIndex][10];
		double lambda_z = dataTable[wavelengthIndex][11];
		
		double solid_angle = 0.0002;
		//Coord3D finalColor = { 0.0f, 0.0f, 0.0f };
		
		double refraction_index;
		double extinction_coeff;
		int conductor_flag;

		switch (SphNum)
		{
			case 0:	//gold
				refraction_index = dataTable[wavelengthIndex][2];
				extinction_coeff = dataTable[wavelengthIndex][3];
				conductor_flag = 1;
				break;
			case 1:	//gold
				refraction_index = dataTable[wavelengthIndex][2];
				extinction_coeff = dataTable[wavelengthIndex][3];
				conductor_flag = 1;
				break;
			case 2:	//gold
				refraction_index = dataTable[wavelengthIndex][2];
				extinction_coeff = dataTable[wavelengthIndex][3];
				conductor_flag = 1;
				break;
			case 3:
				//Aluminium
				//refraction_index = dataTable[wavelengthIndex][4];
				//extinction_coeff = dataTable[wavelengthIndex][5];
				//Copper 
				refraction_index = cu[wavelengthIndex][1];
				extinction_coeff = cu[wavelengthIndex][2];
				conductor_flag = 1;
				break;
			case 4:	//Amorphous Silicon
				refraction_index = dataTable[wavelengthIndex][6];
				extinction_coeff = dataTable[wavelengthIndex][7];
				conductor_flag = 1;
				break;
			case 5:	//Quartz
				refraction_index = dataTable[wavelengthIndex][8];
				extinction_coeff = 0;
				conductor_flag = 0;
				break;
		}
		//double Fresnel_Normal_Incidence;
		//double refraction_index = (1 + sqrt(Fresnel_Normal_Incidence)) / (1 - sqrt(Fresnel_Normal_Incidence));

		//Geometry Factor
		double Geometry_Factor;
		Coord3D H_Vector = { v.x + light.x, v.y + light.y, v.z + light.z };
		normalize(H_Vector.x, H_Vector.y, H_Vector.z);
		double N_H_dot, V_H_dot, N_V_dot, N_L_dot;
		double geometry_shadow, geometry_mask;
		N_H_dot = dot(normal.x, normal.y, normal.z, H_Vector.x, H_Vector.y, H_Vector.z);
		V_H_dot = dot(v.x, v.y, v.z, H_Vector.x, H_Vector.y, H_Vector.z);
		N_V_dot = dot(normal.x, normal.y, normal.z, v.x, v.y, v.z);
		N_L_dot = dot(normal.x, normal.y, normal.z, light.x, light.y, light.z);
		geometry_shadow = 2 * N_H_dot * N_V_dot / V_H_dot;
		geometry_mask = 2 * N_H_dot * N_L_dot / V_H_dot;
		Geometry_Factor = min(1.0, min(geometry_mask, geometry_shadow));

		//Beckmann Distribution Function
		double Distribution_Factor;
		double alpha = angle(normal.x, normal.y, normal.z, H_Vector.x, H_Vector.y, H_Vector.z);
		double term1 = exp(-(pow(tan(alpha) / SphProp[SphNum].roughness, 2)));
		double term2 = pow(SphProp[SphNum].roughness, 2) * pow(cos(alpha), 4);
		Distribution_Factor = term1 / term2;

		//Fresnel Term
		double Fresnel_Factor;
		if (conductor_flag = 0) //Non-Conductors
		{
			double c = V_H_dot;
			double g = sqrt(pow(refraction_index, 2) + pow(c, 2) - 1);
			double term1 = pow((g - c) / (g + c), 2);
			double term2 = pow((c*g + c*c - 1) / (c*g - c*c + 1), 2);
			Fresnel_Factor = 0.5 * term1 * (term2 + 1);
		}
		else //Counductors
		{
			double term1 = pow(refraction_index, 2) + pow(extinction_coeff, 2);
			double term2 = pow(N_L_dot, 2);
			double term3 = 2 * refraction_index * N_L_dot;
			double Fresnel_Factor_1 = (term1 + term2 - term3) / (term1 + term2 + term3);
			double Fresnel_Factor_2 = ((term1*term2) - term3 + 1) / ((term1*term2) + term3 + 1);
			Fresnel_Factor = 0.5 * (Fresnel_Factor_1 + Fresnel_Factor_2);
		}

		//Diffuse Reflectance
		double Diffuse_Reflectance;
		//double Fresnel_Normal = pow((refraction_index - 1) / (refraction_index + 1), 2);
		term1 = pow(extinction_coeff, 2);
		term2 = pow(refraction_index - 1, 2);
		double term3 = pow(refraction_index + 1, 2);
		double Fresnel_Normal = (term1 * 0.2 + term2) / (term1 * 0.2 + term3);
		Diffuse_Reflectance = SphProp[SphNum].diffuse_factor * Fresnel_Normal;

		//Specular Reflectance
		double Specular_Reflectance;
		double specular_component = Fresnel_Factor * Distribution_Factor * Geometry_Factor;
		specular_component /= M_PI * N_L_dot * N_V_dot;
		Specular_Reflectance = SphProp[SphNum].specular_factor * specular_component;

		//Ambient Reflectance
		double Ambient_Reflectance = SphProp[SphNum].ambient_factor * Fresnel_Normal * M_PI;
#if 1
		if (SphNum == 5)
		{
			Diffuse_Reflectance *= 4;
			Specular_Reflectance *= 3;
		}
		else if (SphNum == 4)
			Diffuse_Reflectance *= 0.3;
		else if (SphNum == 3)
		{
			Diffuse_Reflectance *= 0.8;
			Specular_Reflectance *= 0.4;
		}
#endif
		double totalReflectance = (irradiance * 0.05 * Ambient_Reflectance);
		totalReflectance += 0.4 * irradiance * N_L_dot * (Diffuse_Reflectance + 0.1 * Specular_Reflectance);

		XYZ.x += totalReflectance * lambda_x;
		XYZ.y += totalReflectance * lambda_y;
		XYZ.z += totalReflectance * lambda_z;
	}

	//Convert the XYZ to sRGB for drawing
	R = 3.24071 * XYZ.x + (-1.53726) * XYZ.y + (-0.498571) * XYZ.z;
	G = (-0.969258) * XYZ.x + 1.87599 * XYZ.y + 0.0415557 * XYZ.z;
	B = 0.0556352 * XYZ.x + (-0.203996) * XYZ.y + 1.05707 * XYZ.z;
}

// Draws the scene
void drawit(void)
{
	glDrawPixels(ImageW, ImageH, GL_RGB, GL_FLOAT, framebuffer);
	glFlush();
}

// Sets pixel x,y to the color RGB
void setFramebuffer(int x, int y, float R, float G, float B)
{
	if (R <= 1.0)
		if (R >= 0.0)
			framebuffer[y][x][0] = R;
		else
			framebuffer[y][x][0] = 0.0;
	else
		framebuffer[y][x][0] = 1.0;
	if (G <= 1.0)
		if (G >= 0.0)
			framebuffer[y][x][1] = G;
		else
			framebuffer[y][x][1] = 0.0;
	else
		framebuffer[y][x][1] = 1.0;
	if (B <= 1.0)
		if (B >= 0.0)
			framebuffer[y][x][2] = B;
		else
			framebuffer[y][x][2] = 0.0;
	else
		framebuffer[y][x][2] = 1.0;
}

void display(void)
{
	int i, j, k;
	float R, G, B;
	Coord3D refpt;
	Coord3D view;
	Coord3D normal;
	Coord3D light;
	Coord3D intpt;
	double xstep = 12.0 / ImageW;
	double ystep = 8.0 / ImageH;
	double t;
	double a, b, c;
	int intsphere;

	refpt.x = -6.0 + xstep / 2.0;
	refpt.y = -4.0 + ystep / 2.0;
	refpt.z = -10.0;

	for (i = 0;i<ImageW;i++, refpt.x += xstep) 
	{
		for (j = 0;j<ImageH;j++, refpt.y += ystep) 
		{
			// Compute the view vector
			view.x = refpt.x; view.y = refpt.y; view.z = refpt.z;
			normalize(view.x, view.y, view.z);

			// Find intersection with sphere (if any) - only 1 sphere can intesect.
			intsphere = -1;
			for (k = 0;(k<6) && (intsphere == -1);k++) 
			{
				a = 1.0;  // Since normalized;
				b = 2.0*view.x*(-SphCent[k].x) + 2.0*view.y*(-SphCent[k].y) + 2.0*view.z*(-SphCent[k].z);
				c = SphCent[k].x*SphCent[k].x + SphCent[k].y*SphCent[k].y + SphCent[k].z*SphCent[k].z -
					SphRad[k] * SphRad[k];
				if ((b*b - 4 * a * c) >= 0.0) 
				{  
					// We have an intersection with that sphere
					// Want nearest of two intersections
					t = (-b - sqrt(b * b - 4 * a * c)) / 2.0;
					intsphere = k;
				}
			}

			if (intsphere != -1) 
			{ 
				// We had an intersection with a sphere
				intpt.x = t*view.x; intpt.y = t*view.y; intpt.z = t*view.z;
				normal.x = (intpt.x - SphCent[intsphere].x) / SphRad[intsphere];
				normal.y = (intpt.y - SphCent[intsphere].y) / SphRad[intsphere];
				normal.z = (intpt.z - SphCent[intsphere].z) / SphRad[intsphere];
				normalize(normal.x, normal.y, normal.z);

				light.x = Light.x - intpt.x;
				light.y = Light.y - intpt.y;
				light.z = Light.z - intpt.z;
				normalize(light.x, light.y, light.z);
				GetColor(view, normal, light, intsphere, R, G, B);
			}
			else 
			{
				R = G = B = 0.0;
			}
			setFramebuffer(i, j, R, G, B);
		}
		refpt.y = -4.0 + ystep / 2.0;
	}

	drawit();
}

void init(void)
{
	int i, j;

	// Initialize framebuffer to clear
	for (i = 0;i<ImageH;i++) {
		for (j = 0;j<ImageW;j++) {
			framebuffer[i][j][0] = 0.0;
			framebuffer[i][j][1] = 0.0;
			framebuffer[i][j][2] = 0.0;
		}
	}

	// Create Sphere data
	SphCent[0].x = -3.0;
	SphCent[0].y = 1.5;
	SphCent[0].z = -10.0;
	SphProp[0].color = { 1.0, 1.0, 0.0 };
	SphProp[0].ambient_factor = 0.1;
	SphProp[0].diffuse_factor = 0.25;
	SphProp[0].specular_factor = 0.75;
	SphProp[0].roughness = 0.1;

	SphCent[1].x = 0.0;
	SphCent[1].y = 1.5;
	SphCent[1].z = -10.0;
	SphProp[1].color = { 0.0, 1.0, 1.0 };
	SphProp[1].ambient_factor = 0.1;
	SphProp[1].diffuse_factor = 0.3;
	SphProp[1].specular_factor = 0.7;
	SphProp[1].roughness = 0.2;

	SphCent[2].x = 3.0;
	SphCent[2].y = 1.5;
	SphCent[2].z = -10.0;
	SphProp[2].color = { 1.0, 0.0, 1.0 };
	SphProp[2].ambient_factor = 0.1;
	SphProp[2].diffuse_factor = 0.4;
	SphProp[2].specular_factor = 0.6;
	SphProp[2].roughness = 0.3;

	SphCent[3].x = -3.0;
	SphCent[3].y = -1.5;
	SphCent[3].z = -10.0;
	SphProp[3].color = { 1.0, 1.0, 1.0 };
	SphProp[3].ambient_factor = 0.1;
	SphProp[3].diffuse_factor = 0.3;
	SphProp[3].specular_factor = 0.7;
	SphProp[3].roughness = 0.15;

	SphCent[4].x = 0.0;
	SphCent[4].y = -1.5;
	SphCent[4].z = -10.0;
	SphProp[4].color = { 0.5, 0.3, 0.2 };
	SphProp[4].ambient_factor = 0.1;
	SphProp[4].diffuse_factor = 0.7;
	SphProp[4].specular_factor = 0.3;
	SphProp[4].roughness = 0.6;

	SphCent[5].x = 3.0;
	SphCent[5].y = -1.5;
	SphCent[5].z = -10.0;
	SphProp[5].color = { 0.8, 0.2, 0.3 };
	SphProp[5].ambient_factor = 0.1;
	SphProp[5].diffuse_factor = 0.6;
	SphProp[5].specular_factor = 0.4;
	SphProp[5].roughness = 0.15;

	for (i = 0;i<6;i++) SphRad[i] = 1.0;

	// Set Light Position
	Light.x = 10.0;
	Light.y = 10.0;
	Light.z = 10.0;

	// Eye is at origin, looking down -z axis, y axis is up,
	// Looks at 8x6 window centered around z = -10.
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(ImageW, ImageH);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Tushar - 641 Assignment 2");
	init();
	glutDisplayFunc(display);
	glutMainLoop();
	return 0;
}

