#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <glut.h>
#include<glew.h>
#include<iostream>
#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include <MMSystem.h>
#include <mciapi.h>
#include <stdlib.h>
#include <glut.h>
#include<algorithm>
#include <math.h>
#include <random>
#include <Mmsystem.h>
#include <mciapi.h>
#include <playsoundapi.h>
#include <chrono>
#include <cmath>
#include <thread>
#include <string>
#include <set>
#include <deque>
#include <obj.h>
#include <unordered_set> 
#include <iostream>
#include<cstring>  
using namespace std;


static uint64_t splitmix64(uint64_t x) {
	// http://xorshift.di.unimi.it/splitmix64.c
	x += 0x9e3779b97f4a7c15;
	x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
	x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
	return x ^ (x >> 31);
}

uint64_t Hash(uint64_t x) {
	static const uint64_t FIXED_RANDOM = chrono::steady_clock::now().time_since_epoch().count();
	return splitmix64(x + FIXED_RANDOM);
}
class Vector3f {
public:
	float x, y, z;

	Vector3f(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) {
		x = _x;
		y = _y;
		z = _z;
	}

	Vector3f operator+(Vector3f& v) {
		return Vector3f(x + v.x, y + v.y, z + v.z);
	}
	void operator+=(Vector3f& v) {
		x += v.x;
		y += v.y;
		z += v.z;
	}

	Vector3f operator-(Vector3f& v) {
		return Vector3f(x - v.x, y - v.y, z - v.z);
	}
	Vector3f operator*(Vector3f& v) {
		return Vector3f(x * v.x, y * v.y, z * v.z);
	}

	Vector3f operator*(float n) {
		return Vector3f(x * n, y * n, z * n);
	}

	Vector3f operator/(float n) {
		return Vector3f(x / n, y / n, z / n);
	}
	Vector3f operator-(float n) {
		return Vector3f(x - n, y - n, z - n);
	}

	Vector3f unit() {
		return *this / sqrt(x * x + y * y + z * z);
	}
	Vector3f fix() {
		return Vector3f(x, z, y);
	}
	Vector3f cross(Vector3f v) {
		return Vector3f(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
};
int WIDTH = 1450;
int HEIGHT = 750;
int score = 0;
bool movingFront = FALSE;
bool movingBack = FALSE;
bool movingRight = FALSE;
bool movingLeft = FALSE;

bool cameraUp = FALSE;
bool cameraDown = FALSE;
bool cameraRight = FALSE;
bool cameraLeft = FALSE;
int jump = 0;
int view = 1;
double mouseX = 0;
double mouseY = 0;
double sunDim = 0;
double skyDim = 0;
deque <pair<int, int>>takenCoins;
char title[] = "3D Model Loader Sample";

// 3D Projection Options
GLdouble fovy = 45.0;
GLdouble aspectRatio = (GLdouble)WIDTH / (GLdouble)HEIGHT;
GLdouble zNear = 0.1;
GLdouble zFar = 250;
GLdouble angleFront = 0;
GLdouble angleUp = 0;
GLdouble angleCoin = 0;
GLdouble angleSun = 0.04;




Vector3f eye = Vector3f(0, 0.9, 0);
Vector3f center = Vector3f(-1, 0.9, 0);
Vector3f up = Vector3f(0, 1, 0);
Vector3f front = Vector3f(1, 1, 0);

Vector3f player = Vector3f(0, 0, 0);
Vector3f playerV = Vector3f(0, 0, 0);
Vector3f sun = Vector3f(130, 160, 40);
Vector3f enemy = Vector3f(0, 0, -35);
Vector3f enemyNextTarget = Vector3f(0, -1, 0);
double enemySpeed = 0.25;

int cameraZoom = 0;
Model_3DS model_player;
Model_3DS model_enemy;

GLuint tex_sky;
GLTexture tex_sun;
GLuint tex_eye;

GLTexture tex_ground;
Model_3DS model_building1;
Model_3DS model_coin;
obj* O;





int dx[4] = { -1,1,0,0 };
int dy[4] = { 0,0,-1,1 };


int map[15][15] = {
	{1,1,1,0,1,1,1,0,1,1,1,1,0,1,1},
	{0,0,1,0,0,0,0,0,0,0,1,0,0,0,0},
	{1,0,1,1,1,1,0,1,1,0,1,0,1,0,1},
	{1,0,0,0,0,0,0,1,1,0,0,0,1,0,1},
	{0,0,1,1,0,1,0,1,0,0,1,0,0,0,0},
	{1,0,1,1,0,1,0,1,0,1,1,1,0,1,1},
	{1,0,0,0,0,1,0,0,0,1,0,0,0,0,1},
	{1,1,1,0,1,1,1,1,0,1,0,1,1,0,1},
	{1,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
	{0,0,1,1,0,1,0,1,0,1,1,0,1,0,0},
	{1,0,1,1,0,1,0,0,0,1,1,0,1,1,1},
	{1,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
	{1,0,1,1,0,1,1,1,1,0,1,1,1,0,1},
	{0,0,1,0,0,0,0,0,1,0,0,0,0,0,0},
	{1,1,1,0,1,1,1,0,1,1,1,1,0,1,1}
};
int floyd[225][225];

int convert2dTo1d(int x, int y) {
	return x + y * 15;
}
double deg2rad(double degrees) {
	return degrees * 4.0 * atan(1.0) / 180.0;
}
double rad2deg(double rads) {
	return (180.0 * rads) / (4.0 * atan(1.0));
}
void computeFloyd() {
	int V = 225;
	for (int x1 = 0;x1 < 15;x1++) {
		for (int y1 = 0;y1 < 15;y1++) {
			for (int x2 = 0;x2 < 15;x2++) {
				for (int y2 = 0;y2 < 15;y2++) {
					floyd[convert2dTo1d(x1, y1)][convert2dTo1d(x2, y2)] = 1000000;
					if ((abs(x1 - x2) + abs(y1 - y2) == 1)
						&& (map[x1][y1] + map[x2][y2] == 0)) {
						floyd[convert2dTo1d(x1, y1)][convert2dTo1d(x2, y2)] = 1;
					}
					if (x1 == x2 && y1 == y2)floyd[convert2dTo1d(x1, y1)][convert2dTo1d(x2, y2)] = 0;
				}
			}
		}
	}
	for (int k = 0; k < V; k++) {
		for (int i = 0; i < V; i++) {
			for (int j = 0; j < V; j++) {
				if (floyd[i][k] + floyd[k][j] < floyd[i][j])
					floyd[i][j] = floyd[i][k] + floyd[k][j];
			}
		}
	}
}

void InitLightSource()
{
	// Enable Lighting for this OpenGL Program
	glEnable(GL_LIGHTING);
	GLfloat light_position[] = {sun.x, sun.y, sun.z,0 };
	// Enable Light Source number 0
	// OpengL has 8 light sources
	glEnable(GL_LIGHT0);

	// Define Light source 0 ambient light

	/*
	GLfloat ambient[] = { 0.1f, 0.1f, 0.1, 0.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	*/

	// Define Light source 0 diffuse light

	/*
	GLfloat diffuse[] = { 0.5f, 0.5f, 0.5f, 0.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	// Define Light source 0 Specular light

	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	GLfloat lightIntensity[] = { 1.0, 1.0 ,1.0, 0.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity);
	*/



	/// <summary>
	///gLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 30.0);
	//glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 90.0);
	//glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, l0Direction);
	/// </summary>

	// Finally, define light source 0 position in World Space
	glLightfv(GL_LIGHT0,GL_POSITION, light_position);
	
}
void InitMaterial()
{
	// Enable Material Tracking
	glEnable(GL_COLOR_MATERIAL);

	// Sich will be assigneet Material Properties whd by glColor
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	// Set Material's Specular Color
	// Will be applied to all objects
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);

	// Set Material's Shine value (0->128)
	GLfloat shininess[] = { 96.0f };
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}
void myInit(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	gluPerspective(fovy, aspectRatio, zNear, zFar);

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();

	gluLookAt(eye.x, eye.y, eye.z, center.x, center.y, center.z, up.x, up.y, up.z);
}



bool isBuilding(int x, int z) {
	x = round(x / 7.0);
	z = round(z / 7.0);

	if (map[(x % 15 + 16) % 15][(z % 15 + 16) % 15]) {
		return true;
	}
	return false;
}
bool isObsticle(int x, int z) {
	if (x == 0 && z == 0 || isBuilding(x, z) || ((x % 7 == 0) && (z % 7 == 0)))return false;
	if ((Hash(x) + Hash(z)) % 30) {
		return false;
	}
	else {
		if (isObsticle(x + 1, z))return false;
		return true;
	}
}
bool isCoin(int x, int z) {
	return (x % 7 == 0) && (z % 7 == 0) && (find(takenCoins.begin(), takenCoins.end(), (pair<int, int>{x, z})) == takenCoins.end()) && !isBuilding(x, z);
}


void isFreeThenMove(Vector3f acc) {
	if (isBuilding((int)round(player.x + acc.x), (int)round(player.z + acc.z))) {
		sndPlaySound(TEXT("sounds/buildCollision.wav"), SND_ASYNC | SND_FILENAME);
		if (isBuilding((int)round(player.x + acc.x), (int)round(player.z)))
			acc = acc * Vector3f(0, 1, 1);
		if (isBuilding((int)round(player.x + acc.x), (int)round(player.z + acc.z)))
			acc = acc * Vector3f(1, 1, 0);
	}
	if (isObsticle((int)round(player.x + acc.x), (int)round(player.z + acc.z))) {
		if (player.y < 1) {
			if (isObsticle((int)round(player.x + acc.x), (int)round(player.z)))
				acc = acc * Vector3f(0, 1, 1);
			if (isObsticle((int)round(player.x + acc.x), (int)round(player.z + acc.z)))
				acc = acc * Vector3f(1, 1, 0);

		}
	}

	player += acc;

	//sun += Vector3f(acc.x-10, 0, acc.z - 10);
	if (isObsticle((int)round(player.x), (int)round(player.z)) && player.y < 1) {
		player.y = 1;
		playerV.y = 0;
		jump = 0;

	}
	else if (player.y < 0) {
		player.y = 0;
		playerV.y = 0;
		jump = 0;


	}

	if (isCoin((int)round(player.x), (int)round(player.z)) && player.y <= 1) {
		if (takenCoins.size() > 20)takenCoins.pop_front();
		sndPlaySound(TEXT("sounds/collection.wav"), SND_ASYNC | SND_FILENAME);
		takenCoins.push_back(pair<int, int>{(int)round(player.x), (int)round(player.z)});
	}
}
void move() {
	Vector3f acc = Vector3f(0, 0, 0);
	if (movingFront) {
		acc += front.unit();
	}
	if (movingBack) {
		acc += front.unit() * -1;
	}
	Vector3f right = up.cross(front);
	if (movingRight) {
		acc += right.unit() * -1;
	}
	if (movingLeft) {
		acc += right.unit();
	}
	if (acc.x != 0 || acc.y != 0 || acc.z != 0) {
		acc = acc.unit() / 4;
	}
	acc += playerV;
	playerV += Vector3f(0, -0.0098, 0);
	/*
	if (acc.x != 0 || acc.y != 0 || acc.z != 0) {
		player += acc.unit() / 4;
	}
	if (playerV.y != 0) {
		player += playerV;
		playerV += Vector3f(0, -0.0098, 0);
		if (player.y < 0) {
			player.y = 0;
			playerV.y = 0;
		}
	}
	*/

	isFreeThenMove(acc);
}
void moveEnemy() {
	int xp = (int)round(player.x / 7.0) + 1;
	int yp = (int)round(player.z / 7.0) + 1;
	int xe = (int)round(enemy.x / 7.0) + 1;
	int ye = (int)round(enemy.z / 7.0) + 1;
	int Xp = xp / 15 - (((xp % 15 != 0) && (xp < 0)) ? 1 : 0);
	int Yp = yp / 15 - (((yp % 15 != 0) && (yp < 0)) ? 1 : 0);
	int Xe = xe / 15 - (((xe % 15 != 0) && (xe < 0)) ? 1 : 0);
	int Ye = ye / 15 - (((ye % 15 != 0) && (ye < 0)) ? 1 : 0);
	if (xp == xe && yp == ye) {
		enemy += (player - enemy).unit() * Vector3f(1, 0, 1) * enemySpeed;
	}
	else {

		if ((enemyNextTarget.y < 0) && ((Xp != Xe) || (Yp != Ye))) {
			if (((abs(Xp - Xe) < abs(Yp - Ye)) && (Xp != Xe)) || (Yp == Ye)) {
				if (player.x > enemy.x) {
					xp = 14;
					yp = 7;
				}
				else {
					xp = 0;
					yp = 7;
				}
			}
			else {

				if (player.z > enemy.z) {
					xp = 4;
					yp = 14;
				}
				else {
					xp = 4;
					yp = 0;
				}
			}
			if (((xe % 15 + 15) % 15 == (xp % 15 + 15) % 15) && ((ye % 15 + 15) % 15 == (yp % 15 + 15) % 15)) {
				if (xp == 0) {
					enemyNextTarget = Vector3f((xe - 1 - 1) * 7, 1, (ye - 1) * 7);
				}
				else if (xp == 14) {
					enemyNextTarget = Vector3f((xe - 1 + 1) * 7, 1, (ye - 1) * 7);

				}
				else if (yp == 0) {
					enemyNextTarget = Vector3f((xe - 1) * 7, 1, (ye - 1 - 1) * 7);
				}
				else if (yp == 14) {
					enemyNextTarget = Vector3f((xe - 1) * 7, 1, (ye - 1 + 1) * 7);

				}
			}

		}

		if (enemyNextTarget.y < 0) {

			int bestInd = -1;
			int bestV = 1000000000;
			for (int i = 0;i < 4;i++) {
				if (((xe % 15 + 15) % 15 + dx[i] < 0) || ((xe % 15 + 15) % 15 + dx[i] > 14)
					|| ((ye % 15 + 15) % 15 + dy[i] < 0) || ((ye % 15 + 15) % 15 + dy[i] > 14)
					|| map[(xe % 15 + 15) % 15 + dx[i]][(ye % 15 + 15) % 15 + dy[i]])continue;


				if (floyd[convert2dTo1d((xe % 15 + dx[i] + 15) % 15, (ye % 15 + dy[i] + 15) % 15)][convert2dTo1d((xp % 15 + 15) % 15, (yp % 15 + 15) % 15)] < bestV) {
					bestV = floyd[convert2dTo1d((xe % 15 + dx[i] + 15) % 15, (ye % 15 + dy[i] + 15) % 15)][convert2dTo1d((xp % 15 + 15) % 15, (yp % 15 + 15) % 15)];
					bestInd = i;
				}
			}

			enemyNextTarget = Vector3f((xe - 1 + dx[bestInd]) * 7, 1, (ye - 1 + dy[bestInd]) * 7);
		}
		enemy += (enemyNextTarget - enemy).unit() * Vector3f(1, 0, 1) * enemySpeed;
		if (abs(enemy.x - enemyNextTarget.x) + abs(enemy.z - enemyNextTarget.z) <= 1) {
			enemyNextTarget.y = -1;
		}
	}

	if (abs(player.x - enemy.x) + abs(player.z - enemy.z) > 50)enemySpeed = 1;
	if (abs(player.x - enemy.x) + abs(player.z - enemy.z) < 28)enemySpeed = 0.25;

}
void rotateSun() {

	sun = Vector3f(cos(deg2rad(angleSun)) * sun.x - sin(deg2rad(angleSun)) * sun.y, sin(deg2rad(angleSun)) * sun.x + cos(deg2rad(angleSun)) * sun.y, sun.z);
}







void drawCoin(int x, int z) {
	glPushMatrix();
	glTranslatef(0, -0.4, 0);
	glRotatef(angleCoin + (x + 1) * (z + 1) * 7, 0, 1, 0);
	glRotatef(90, 1, 0, 0);
	glScalef(0.6, 0.6, 0.6);
	model_coin.Draw();
	glPopMatrix();
}
void drawSun() {
	glDisable(GL_LIGHTING);
	glPushMatrix();
	glTranslatef(sun.x + player.x, sun.y, sun.z + player.z);
	glColor3f(1, 1+sunDim, 0); //dim 
	GLUquadricObj* qfoot;
	qfoot = gluNewQuadric();
	glBindTexture(GL_TEXTURE_2D, tex_sun.texture[0]);
	gluQuadricNormals(qfoot, GL_SMOOTH);
	gluQuadricTexture(qfoot, GL_TRUE);
	gluSphere(qfoot, 20, 20, 20);
	gluDeleteQuadric(qfoot);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	glEnable(GL_LIGHTING);

}
void drawSphere(double x, double y, double z,double r, GLuint tex) {
	glDisable(GL_LIGHTING);
	glPushMatrix();
	GLUquadricObj* qobj;

	qobj = gluNewQuadric();
	glTranslated(x, y, z);
	glRotated(-90, 1, 0, 0);
	glBindTexture(GL_TEXTURE_2D, tex);
	gluQuadricTexture(qobj, true);
	gluQuadricNormals(qobj, GL_SMOOTH);
	gluSphere(qobj, r, 100, 100);
	gluDeleteQuadric(qobj);
	glPopMatrix();
	glEnable(GL_LIGHTING);

}
void drawCircle(int x, int y, float r, bool solid) {
	glPushMatrix();
	glTranslatef(x, y, 0);
	GLUquadric* quadObj = gluNewQuadric();
	gluDisk(quadObj, solid ? 0 : r - 4, r, 50, 50);
	glPopMatrix();

}


void RenderCoins()

{



	glColor3f(1, 1, 1);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);	// Bind the ground texture
	int centerx = (int)(player.x / 7) * 7;
	int centerz = (int)(player.z / 7) * 7;


	for (int x = -28 + centerx;x - centerx <= 28;x += 7) {
		for (int z = -28 + centerz;z - centerz <= 28;z += 7) {
			if (isCoin(x, z)) {
				glPushMatrix();
				glTranslatef(x, 0.501, z);
				drawCoin(x, z);
				glPopMatrix();
			}
		}
	}




	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}
void RenderMap()

{


	glColor3f(1, 1, 1);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);	// Bind the ground texture
	int centerx = round(player.x / 7.0);
	int centerz = round(player.z / 7.0);


	for (int x = -5 + centerx;x - centerx <= 5;x++) {


		for (int z = -5 + centerz;z - centerz <= 5;z++) {
			if (abs(centerx - x) > 3 && abs(centerz - z) > 3)continue;

			if (map[(x % 15 + 16) % 15][(z % 15 + 16) % 15]) {
				glPushMatrix();
				glTranslatef(x * 7, 0, z * 7);
				//glColor3f(0, 0, 1);
				glScalef(0.004, 0.004, 0.0043);
				model_building1.Draw();
				glPopMatrix();
			}
		}
	}




	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}
void RenderObsticles()

{



	glColor3f(1, 1, 1);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);	// Bind the ground texture
	int centerx = ((int)player.x / 4) * 4;
	int centerz = ((int)player.z / 4) * 4;


	for (int x = -30 + centerx;x - centerx <= 30;x++) {


		for (int z = -30 + centerz;z - centerz <= 30;z++) {
			if (isObsticle(x, z)) {
				glPushMatrix();
				glTranslatef(x, 0.501, z);
				glColor3f(0, 0, 0);
				glutSolidCube(1);
				//drawCoin();
				glPopMatrix();
			}
		}
	}



	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}
void RenderGround()

{


	glColor3f(1, 1, 1);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);	// Bind the ground texture
	int centerx = ((int)player.x / 4) * 4;
	int centerz = ((int)player.z / 4) * 4;

	for (int x = -40 + centerx;x - centerx <= 40;x += 4) {
		for (int z = -40 + centerz;z - centerz <= 40;z += 4) {
			glPushMatrix();
			glBegin(GL_QUADS);
			glNormal3f(0, 1, 0);	// Set quad normal direction.
			glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
			glVertex3f(-2 + x, 0, -2 + z);
			glTexCoord2f(1, 0);
			glVertex3f(2 + x, 0, -2 + z);
			glTexCoord2f(1, 1);
			glVertex3f(2 + x, 0, 2 + z);
			glTexCoord2f(0, 1);
			glVertex3f(-2 + x, 0, 2 + z);
			glEnd();
			glPopMatrix();
		}
	}



	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}
void RenderEnemy() {
	double dot = player.z - enemy.z;
	double det = player.x - enemy.x;
	double angle = rad2deg(atan2(det, dot));
	glPushMatrix();
	glPushMatrix();
	glTranslated(enemy.x, enemy.y, enemy.z - 0.02);
	glRotatef(angle, 0, 1, 0);
	drawSphere(0.38, 1.48, 2.1, 0.08, tex_eye);
	drawSphere(-0.38, 1.48, 2.1, 0.08, tex_eye);
	glScalef(0.15, 0.15, 0.15);

	model_enemy.Draw();
	//obj_render(spider);
	glPopMatrix();
	glPopMatrix();

}
void RenderPlayer() {
	glPushMatrix();
	glPushMatrix();
	glTranslated(player.x, player.y, player.z - 0.02);
	glRotatef(angleFront + 90, 0, 1, 0);
	glScalef(0.015, 0.015, 0.015);
	model_player.Draw();

	//obj_render(spider);
	glPopMatrix();
	glPopMatrix();

}

void Special(int key, int x, int y) {

	switch (key) {
	case GLUT_KEY_UP: {
		cameraUp = true;
		break;
	}
	case GLUT_KEY_DOWN: {
		cameraDown = true;
		break;}
	case GLUT_KEY_LEFT: {
		cameraLeft = true;
		break;
	}
	case GLUT_KEY_RIGHT: {
		cameraRight = true;
		break;
	}
	}

	glutPostRedisplay();
}
void SpecialUp(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP: {
		cameraUp = false;
		break;
	}
	case GLUT_KEY_DOWN: {
		cameraDown = false;
		break;}
	case GLUT_KEY_LEFT: {
		cameraLeft = false;
		break;
	}
	case GLUT_KEY_RIGHT: {
		cameraRight = false;
		break;
	}
	}
	glutPostRedisplay();
}
void myKeyboard(unsigned char button, int x, int y)

{

	switch (button)
	{
	case ' ': {
		if (jump < 100) {
			sndPlaySound(TEXT("sounds/wallCollision.wav"), SND_ASYNC | SND_FILENAME);
			playerV += Vector3f(0, 0.2, 0);
			jump++;
		}
		break;
	}
	case 'w': {
		movingFront = true;
		break;
	}
	case 's': {
		movingBack = true;
		break;}
	case 'a': {
		movingLeft = true;
		break;
	}
	case 'd': {
		movingRight = true;
		break;
	}
	case '1':
		//First person
		view = 1;
		break;
	case '2':
		//second person
		view = 2;
		break;
	case '3':
		//Third person
		view = 3;
		break;

	case '9':
		exit(0);
		break;
	default:
		break;
	}


	glLoadIdentity();

	gluLookAt(eye.x, eye.y, eye.z, center.x, center.y, center.z, up.x, up.y, up.z);



	glutPostRedisplay();
}
void myKeyboardUp(unsigned char button, int x, int y) {
	switch (button)
	{
	case 'w': {
		movingFront = false;
		break;
	}
	case 's': {
		movingBack = false;
		break;}
	case 'a': {
		movingLeft = false;
		break;
	}
	case 'd': {
		movingRight = false;
		break;
	}
	default:
		break;
	}
}
void myMouse(int x, int y)
{
	angleFront += (500 - x) / 8;
	if (angleUp + (y - 500) / 8 > -90 && angleUp + (y - 500) / 8 < 90)
		angleUp += (y - 500) / 8;

	if (angleFront >= 360) {
		angleFront -= 360;
	}
	if (angleFront < 0) {
		angleFront += 360;
	}

	SetCursorPos(500, 500);
}

void print(Vector3f pos, string string)
{	
	int len, i;

	glRasterPos3f(pos.x, pos.y, pos.z);
	len = string.size();
	for (i = 0; i < len; i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
	}

}
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ShowCursor(false);
	InitLightSource();
	InitMaterial();

	glColor3f(0, 0, 0);

	drawSun();
	RenderGround();// Draw Ground
	RenderObsticles();
	RenderCoins();
	RenderMap();
	RenderEnemy();
	if (view != 1)
		RenderPlayer();

	glPushMatrix();
	GLUquadricObj* qobj;
	qobj = gluNewQuadric();
	glTranslated(player.x, 0, player.z);
	glColor3f(0.7+skyDim, 0.7+skyDim, 0.7+skyDim);
	glRotated(90, 1, 0, 1);
	glBindTexture(GL_TEXTURE_2D, tex_sky);
	gluQuadricTexture(qobj, true);
	gluQuadricNormals(qobj, GL_SMOOTH);
	gluSphere(qobj, 200, 100, 100);
	gluDeleteQuadric(qobj);
	glPopMatrix();

	
	glutSwapBuffers();
}
void tick(int value) {
	move();
	moveEnemy();
	if (sunDim >= -0.7) {

	sunDim -= 0.001;
	}
	if (skyDim >= -0.5) {
		skyDim -= 0.001;
	}
	rotateSun();
	if (cameraUp && (angleUp - 4 > -90)) {
		angleUp -= 4;
	}
	if (cameraDown && (angleUp + 4 < 90)) {
		angleUp += 4;
	}
	if (cameraRight) {
		angleFront -= 4;
	}
	if (cameraLeft) {
		angleFront += 4;
	}

	front = Vector3f(cos(deg2rad(angleFront)), 0, -sin(deg2rad(angleFront)));

	if (view == 1) {

		eye = Vector3f(0, 1.2, 0) + player;
		center = Vector3f(cos(deg2rad(angleFront)) * cos(deg2rad(angleUp)), -sin(deg2rad(angleUp)), -sin(deg2rad(angleFront)) * cos(deg2rad(angleUp))) * 300 + Vector3f(0, 0.9, 0) + player;
	}
	else {
		center = Vector3f(cos(deg2rad(angleFront)) * cos(deg2rad(angleUp)), -sin(deg2rad(angleUp)), -sin(deg2rad(angleFront)) * cos(deg2rad(angleUp))) * 3 + Vector3f(0, 1, 0) + player;
		eye = player * 2 + Vector3f(0, 2, 0) - center;
		if (eye.y < 0) {
			eye += (center - eye) * (-eye.y / (center.y - eye.y));
		}

		if (view == 2) {
			eye += (player - eye) * Vector3f(1, 0, 1) * 2;
			center += (player - center) * Vector3f(1, 0, 1) * 2;
		}
		if (isBuilding((int)round(eye.x), (int)round(eye.z))) {
			Vector3f inc = (center - eye) / 2;
			for (int i = 0;i < 10;i++) {
				if (isBuilding((int)round(eye.x), (int)round(eye.z)))
					eye += inc;
				else
					eye += inc * -1;
				inc = inc / 2;
			}
		}
	}

	angleCoin += 3;
	if (angleCoin >= 360)angleCoin -= 360;


	glLoadIdentity();

	gluLookAt(eye.x, eye.y, eye.z, center.x, center.y, center.z, up.x, up.y, up.z);
	glutPostRedisplay();	//Re-draw scene 
	glutTimerFunc(20, tick, 0);

}


void myReshape(int w, int h)
{
	if (h == 0) {
		h = 1;
	}

	WIDTH = w;
	HEIGHT = h;

	// set the drawable region of the window
	glViewport(0, 0, w, h);

	// set up the projection matrix 
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, (GLdouble)WIDTH / (GLdouble)HEIGHT, zNear, zFar);

	// go back to modelview matrix so we can move the objects about
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eye.x, eye.y, eye.z, center.x, center.y, center.z, center.x, center.y, center.z);
}
void LoadAssets()
{
	model_player.Load("Models/player/man.3ds");
	model_enemy.Load("models/enemy/2.3ds");

	model_building1.Load("Models/building1/Tower Constantino Eleninskaya Kremlin N120615.3DS");
	model_coin.Load("Models/gold/gold.3ds");
	tex_ground.Load("Textures/street.bmp");
	tex_sun.Load("Textures/sun.bmp");

	loadBMP(&tex_eye, "Models/enemy/eye.bmp", true);

	loadBMP(&tex_sky, "Textures/blu-sky-3.bmp", true);
	O = obj_create("D:\Downloads\bge\Alien+Animal+Actions_Baked_BGE.obj");

}
void main(int argc, char** argv)
{
	computeFloyd();
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(WIDTH, HEIGHT);

	glutInitWindowPosition(50, 50);

	glutCreateWindow(title);

	glutDisplayFunc(display);

	glutKeyboardFunc(myKeyboard);
	glutKeyboardUpFunc(myKeyboardUp);
	glutPassiveMotionFunc(myMouse);


	glutSpecialFunc(Special);
	glutSpecialUpFunc(SpecialUp);

	glutReshapeFunc(myReshape);

	myInit();

	glutTimerFunc(10, tick, 0);

	LoadAssets();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);
	glutFullScreen();
	glShadeModel(GL_SMOOTH);
	glutMainLoop();
}