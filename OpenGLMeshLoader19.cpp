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

void renderPlayer();

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
	double dis(Vector3f v) {
		return sqrt((x - v.x) * (x - v.x) + (y - v.y) * (y - v.y) + (z - v.z) * (z - v.z));
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
int view = 3;
int health = 3;
int playerFrame = 5;
int frameDirection = 1;
int playerDirection = 0;
double mouseX = 0;
double mouseY = 0;
double sunDim = 0;
double skyDim = 0;
double light1 = 0;
double flicker = 1;
double toFlicker = 50;
double isCompleted = false;
bool wonEndGame = false;
bool isNight = false;
double loadingPower = 0;
double playerSpeed = 1;
double speedCount = 300;
bool isFalling = false;
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
Vector3f target = Vector3f(((Hash(rand()) % 2 + 1) * 15 + 5) * 7, 0, ((Hash(rand()) % 2 + 1) * 15 + 6) * 7);

Vector3f sun = Vector3f(130, 160, 40);
Vector3f enemy = Vector3f(0, 0, -35);
Vector3f enemyNextTarget = Vector3f(0, -1, 0);
double enemySpeed = 0.25;


Model_3DS model_player;
Model_3DS model_enemy;
Model_3DS model_building1;
Model_3DS model_coin;
Model_3DS model_target;
Model_3DS model_can;
Model_3DS model_car;
Model_3DS model_lightPost;
Model_3DS model_drink;
vector<Model_3DS> model_character;


GLuint tex_sky;
GLuint tex_sky_night;
GLuint tex_eye;

GLTexture tex_sun;
GLTexture tex_moon;
GLTexture tex_road;
GLTexture tex_ground;




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
int obsticleMap[5][7][7]{
	{
		{0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0}
	},
	{
		{0,0,0,0,0,0,0},
		{0,1,1,0,0,0,0},
		{0,1,1,0,0,0,0},
		{0,1,1,0,0,0,0},
		{0,1,1,0,0,0,0},
		{0,1,1,0,0,0,0},
		{0,0,0,0,0,0,0}
	},
	{
		{0,0,1,1,1,1,1},
		{0,0,1,1,1,1,1},
		{0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0},
		{1,1,1,1,1,0,0},
		{1,1,1,1,1,0,0},
		{0,0,0,0,0,0,0}
	},
	{
		{0,0,0,0,0,0,0},
		{0,1,1,1,1,1,0},
		{0,1,1,1,1,1,0},
		{0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0}
	},
	{
		{0,0,0,0,0,0,0},
		{1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1},
		{1,1,0,0,0,0,0},
		{1,1,0,0,0,0,0},
		{1,1,0,0,0,0,0},
		{0,0,0,0,0,0,0}
	}
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

	GLfloat lmodel_ambient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

	if (isNight) {
		//camera lights
		glEnable(GL_LIGHT0);
		glEnable(GL_LIGHT1);
		glEnable(GL_LIGHT2);
		glEnable(GL_LIGHT3);
		glEnable(GL_LIGHT4);
		glEnable(GL_LIGHT5);
		glEnable(GL_LIGHT6);
		glEnable(GL_LIGHT7);
		bool s = true;
		GLfloat l0Diffuse[] = { 0.7f, 0.7f, 0.7f, 1.0f };
		GLfloat l0Spec[] = { 1.0f, 1.0f, 0.0f, 1.0f };
		GLfloat l0Ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
		GLfloat l0Position[] = { eye.x, eye.y, eye.z, s };
		GLfloat l0Direction[] = { (center - eye).x, (center - eye).y,(center - eye).z };
		glLightfv(GL_LIGHT0, GL_DIFFUSE, l0Diffuse);
		glLightfv(GL_LIGHT0, GL_POSITION, l0Position);
		glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, l0Direction);


	}
	else {
		//camera lights
		glEnable(GL_LIGHT0);
		glEnable(GL_LIGHT1);
		glDisable(GL_LIGHT2);
		glDisable(GL_LIGHT3);
		glDisable(GL_LIGHT4);
		glDisable(GL_LIGHT5);
		glDisable(GL_LIGHT6);
		glDisable(GL_LIGHT7);
		GLfloat light0Intensity[] = { 1.0 + light1, 1.0 + light1 ,1.0 + light1, 1.0f };
		GLfloat light0_Position[] = { 0.0f,0.0f, 0.0f, 0.0f };

		glLightfv(GL_LIGHT0, GL_DIFFUSE, light0Intensity);
		glLightfv(GL_LIGHT0, GL_POSITION, light0_Position);


		//sun lights
		GLfloat light1Intensity[] = { 1.0 , 1.0 + sunDim ,0.93 + sunDim, 1.0f };
		GLfloat light1_position[] = { sun.x, sun.y, sun.z,0 };
		glLightfv(GL_LIGHT1, GL_AMBIENT, light1Intensity);
		glLightfv(GL_LIGHT1, GL_POSITION, light1_position);

	}

	glDisable(GL_LIGHTING);


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
void endGame() {
	isCompleted = true;
	view = 3;
}
void restart(bool youWon) {


	player = Vector3f(0, 0, 0);
	playerV = Vector3f(0, 0, 0);
	target = Vector3f(((Hash(rand()) % 2 + 1) * 15 + 5) * 7, 0, ((Hash(rand()) % 2 + 1) * 15 + 6) * 7);
	sun = Vector3f(130, 160, 40);
	enemy = Vector3f(0, 0, -35);
	enemyNextTarget = Vector3f(0, -1, 0);
	isFalling = false;
	enemySpeed = 0.25;
	if (youWon) {
		if (isNight) {
			sndPlaySound(TEXT("sounds/endGame.wav"), SND_ASYNC | SND_FILENAME);
			wonEndGame = true;
			endGame();
		}
		else {
			isNight = true;
			sunDim = 0;
			skyDim = 0;
			light1 = 0;
		}

	}
	else {
		health--;
	}
	if (health == 0) {
		sndPlaySound(TEXT("sounds/lose.wav"), SND_ASYNC | SND_FILENAME);

		wonEndGame = false;
		endGame();
	}
}
void startGame() {
	 score = 0;
	 movingFront = FALSE;
	 movingBack = FALSE;
	 movingRight = FALSE;
	 movingLeft = FALSE;
	 cameraUp = FALSE;
	 cameraDown = FALSE;
	 cameraRight = FALSE;
	 cameraLeft = FALSE;
	 jump = 0;
	 view = 3;
	 health = 3;
	 playerFrame = 5;
	 frameDirection = 1;
	 playerDirection = 0;
	 mouseX = 0;
	 mouseY = 0;
	 sunDim = 0;
	 skyDim = 0;
	 light1 = 0;
	 flicker = 1;
	 toFlicker = 50;
	 isCompleted = false;
	 wonEndGame = false;
	 isNight = false;
	 loadingPower = 0;
	 playerSpeed = 1;
	 speedCount = 300;
	 isFalling = false;
	// 3D Projection Options
	 fovy = 45.0;
	 aspectRatio = (GLdouble)WIDTH / (GLdouble)HEIGHT;
	 zNear = 0.1;
	 zFar = 250;
	 angleFront = 0;
	 angleUp = 0;
     angleCoin = 0;
	 angleSun = 0.04;
	 eye = Vector3f(0, 0.9, 0);
	 center = Vector3f(-1, 0.9, 0);
	 up = Vector3f(0, 1, 0);
	 front = Vector3f(1, 1, 0);
	 player = Vector3f(0, 0, 0);
	 playerV = Vector3f(0, 0, 0);
     target = Vector3f(((Hash(rand()) % 3 + 2) * 15 + 5) * 7, 0, ((Hash(rand()) % 3 + 2) * 15 + 6) * 7);
	 sun = Vector3f(130, 160, 40);
     enemy = Vector3f(0, 0, -35);
	 enemyNextTarget = Vector3f(0, -1, 0);
	 enemySpeed = 0.25;
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
	if (isBuilding(x, z))return false;
	int centerx = (int)round(x / 7.0);
	int centerz = (int)round(z / 7.0);
	return obsticleMap[((centerx + centerz) % 5 + 5) % 5][x - centerx * 7 + 3][z - centerz * 7 + 3];

}
bool isCoin(int x, int z) {
	return (x % 7 == 0) && (z % 7 == 0) && (x != target.x || z != target.z) && ((Hash(x) + Hash(z)) % 10 == 0) && (find(takenCoins.begin(), takenCoins.end(), (pair<int, int>{x, z})) == takenCoins.end()) && !isBuilding(x, z);
}
bool isLightPost(int x, int z) {
	if (x == 0 && z == 0)return false;
	return (x % 7 == 0) && (z % 7 == 0) && !isBuilding(x, z);
}


void isFreeThenMove(Vector3f acc) {
	if (isBuilding((int)round(player.x + acc.x), (int)round(player.z + acc.z))) {
		if (isBuilding((int)round(player.x + acc.x), (int)round(player.z)))
			acc = acc * Vector3f(0, 1, 1);
		if (isBuilding((int)round(player.x + acc.x), (int)round(player.z + acc.z)))
			acc = acc * Vector3f(1, 1, 0);
	}
	if (isObsticle((int)round(player.x + acc.x), (int)round(player.z + acc.z)) && isNight) {

		if (player.y < 2) {
			if (isObsticle((int)round(player.x + acc.x), (int)round(player.z))) {
				/*if (isNight) {
				  PlaySound(TEXT("sounds/carCollision.wav"),NULL, SND_ASYNC | SND_FILENAME);

				}*/
				acc = acc * Vector3f(0, 1, 1);
			}
			if (isObsticle((int)round(player.x + acc.x), (int)round(player.z + acc.z)))
				/*if (isNight) {
					PlaySound(TEXT("sounds/carCollision.wav"), NULL,SND_ASYNC | SND_FILENAME);
				}*/
				acc = acc * Vector3f(1, 1, 0);


		}
	}

	player += acc * (playerSpeed);

	//sun += Vector3f(acc.x-10, 0, acc.z - 10);
	if (isObsticle((int)round(player.x), (int)round(player.z))) {
		if (isNight && (player.y < 2.1)) {
			player.y = 2.1;
			playerV.y = 0;
			jump = 0;
		}
		else if ((player.y < -0.3) && (!isFalling)) {
			isFalling = true;
			PlaySound(TEXT("sounds/falling.wav"), NULL, SND_FILENAME | SND_ASYNC);

		}
	}
	else if (player.y < 0 && (!isFalling)) {
		player.y = 0;
		playerV.y = 0;
		jump = 0;


	}


	if (isCoin((int)round(player.x), (int)round(player.z)) && player.y <= 1) {
		if (takenCoins.size() > 20)takenCoins.pop_front();
		if (isNight) {
			sndPlaySound(TEXT("sounds/drink.wav"), SND_ASYNC | SND_FILENAME);

		}
		else {
			sndPlaySound(TEXT("sounds/collection.wav"), SND_ASYNC | SND_FILENAME);

		}
		score++;
		if (playerSpeed == 1) {
			loadingPower++;

		}
		takenCoins.push_back(pair<int, int>{(int)round(player.x), (int)round(player.z)});
	}

	if (acc.x == 0 && acc.z == 0) {
		playerFrame = 5;
		frameDirection = 1;
	}
	else {
		playerFrame += frameDirection;
		if (playerFrame == 0 || playerFrame == model_character.size() - 1)
		{
			frameDirection *= -1;
		}
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
	playerV += Vector3f(0, -0.01, 0);

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
	if (isNight) {
		glDisable(GL_LIGHTING);
		glTranslatef(0, -0.5, 0);
		glRotatef(angleCoin + (x + 1) * (z + 1) * 7, 0, 1, 0);
		glRotatef(90, 1, 0, 0);
		glScalef(0.15, 0.15, 0.15);
		model_drink.Draw();
		glEnable(GL_LIGHTING);
	}
	else {
		glTranslatef(0, -0.4, 0);
		glRotatef(angleCoin + (x + 1) * (z + 1) * 7, 0, 1, 0);
		glRotatef(90, 1, 0, 0);
		glScalef(0.6, 0.6, 0.6);
		model_coin.Draw();
	}

	glPopMatrix();
}
void drawSun() {
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	glTranslatef(sun.x + player.x, sun.y, sun.z + player.z);
	if (isNight) {
		glColor3f(1, 1, 1);
	}
	else {

		glColor3f(1, 1 + sunDim, 0); //dim 
	}
	GLUquadricObj* qfoot;
	qfoot = gluNewQuadric();
	if (isNight) {
		glBindTexture(GL_TEXTURE_2D, tex_moon.texture[0]);

	}
	else {
		glBindTexture(GL_TEXTURE_2D, tex_sun.texture[0]);

	}
	gluQuadricNormals(qfoot, GL_SMOOTH);
	gluQuadricTexture(qfoot, GL_TRUE);
	gluSphere(qfoot, 20, 20, 20);
	gluDeleteQuadric(qfoot);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);

}
void drawSphere(double x, double y, double z, double r, GLuint tex) {
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
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
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

}
void drawCircle(Vector3f pos, float ir, float r) {
	glPushMatrix();
	glTranslatef(pos.x, pos.y, pos.z);
	glRotatef(90, 0, 1, 0);
	GLUquadric* quadObj = gluNewQuadric();
	gluDisk(quadObj, ir, r, 50, 50);
	glPopMatrix();

}
void drawTriangle(double x1, double y1, double x2, double y2, double x3, double y3) {
	glBegin(GL_TRIANGLES);
	glVertex3f(0, y1, x1);
	glVertex3f(0, y2, x2);
	glVertex3f(0, y3, x3);
	glEnd();
}
void drawHealth(int x, int y) {
	glPushMatrix();
	glScalef(0.0003, 0.0003, 0.0003);
	drawCircle(Vector3f(0, y + 14.8, x - 4.9), 0, 5.22);
	drawCircle(Vector3f(0, y + 14.8, x + 4.9), 0, 5.22);
	drawTriangle(x, y, x - 9.8, y + 13, x + 9.8, y + 13);
	glPopMatrix();
}
void drawLightPost() {
	glDisable(GL_LIGHTING);
	glPushMatrix();
	glScalef(0.01, 0.01, 0.01);
	glColor3f(0.2, 0.2, flicker);
	glRotated(-45, 0, 1, 0);
	model_lightPost.Draw();
	glPopMatrix();
	glEnable(GL_LIGHTING);


}
void drawCar(double x,double z,bool isRotated) {
	glPushMatrix();
	glColor3f(0.7, 0.7, 0.7);
	glTranslatef(x, 0, z);
	if (isRotated) {
		glRotatef(90, 0, 1, 0);
	}
	glTranslatef(0, 0, 0.35);
	glScalef(0.035, 0.035, 0.03);
	model_car.Draw();
	glPopMatrix();
}
void print(Vector3f pos, string string) {
	int len, i;
	glRasterPos3f(pos.x, pos.y, pos.z);
	len = string.size();
	for (i = 0; i < len; i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
	}
}

void renderTarget() {
	glPushMatrix();
	glColor3f(1, 1, 1);
	glTranslatef(target.x, 0, target.z);
	glScalef(0.015, 0.015, 0.015);
	glRotatef(angleCoin, 0, 1, 0);
	model_target.Draw();
	glPopMatrix();
}
void renderCoins()

{




	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing
	glColor3f(1, 1, 1);

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
	glDisable(GL_TEXTURE_2D);
}
void renderLightPosts()

{


	int centerx = ((int)player.x / 7) * 7;
	int centerz = ((int)player.z / 7) * 7;
	vector<Vector3f> illumination;

	for (int x = -28 + centerx;x - centerx <= 28;x += 7) {
		for (int z = -28 + centerz;z - centerz <= 28;z += 7) {
			if (isLightPost(x, z)) {
				glPushMatrix();
				glTranslatef(x - 3, 0, z - 3);
				drawLightPost();
				glPopMatrix();
				if (abs(centerx - x) + abs(centerz - z) <= 14) {
					illumination.push_back(Vector3f(x - 3, 20, z - 3));
				}

			}
		}
	}
	GLfloat lDirection[] = { 0, -1,0 };
	GLfloat lDiffuse[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	for (int i = 0; i < illumination.size();i++) {
		if (i == 0) {
			GLfloat l1internsity[] = { 0.0f,0.0f,0.0f,0.0f };
			GLfloat l1Position[] = { illumination[i].x,illumination[i].y,illumination[i].z, flicker };
			glLightfv(GL_LIGHT1, GL_POSITION, l1Position);
			glLightfv(GL_LIGHT1, GL_AMBIENT, l1internsity);
			glLightfv(GL_LIGHT1, GL_DIFFUSE, lDiffuse);
			glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 30.0);
			glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 90.0);
			glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, lDirection);
		}
		else if (i == 1) {
			GLfloat l2Position[] = { illumination[i].x,illumination[i].y,illumination[i].z, flicker };
			glLightfv(GL_LIGHT2, GL_POSITION, l2Position);
			glLightfv(GL_LIGHT2, GL_DIFFUSE, lDiffuse);
			glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 30.0);
			glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 90.0);
			glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, lDirection);
		}
		else if (i == 2) {
			GLfloat l3Position[] = { illumination[i].x,illumination[i].y,illumination[i].z, flicker };
			glLightfv(GL_LIGHT3, GL_POSITION, l3Position);
			glLightfv(GL_LIGHT3, GL_DIFFUSE, lDiffuse);
			glLightf(GL_LIGHT3, GL_SPOT_CUTOFF, 30.0);
			glLightf(GL_LIGHT3, GL_SPOT_EXPONENT, 90.0);
			glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, lDirection);
		}
		else if (i == 3) {
			GLfloat l4Position[] = { illumination[i].x,illumination[i].y,illumination[i].z, flicker };
			glLightfv(GL_LIGHT4, GL_POSITION, l4Position);
			glLightfv(GL_LIGHT4, GL_DIFFUSE, lDiffuse);
			glLightf(GL_LIGHT4, GL_SPOT_CUTOFF, 30.0);
			glLightf(GL_LIGHT4, GL_SPOT_EXPONENT, 90.0);
			glLightfv(GL_LIGHT4, GL_SPOT_DIRECTION, lDirection);
		}
		else if (i == 4) {
			GLfloat l5Position[] = { illumination[i].x,illumination[i].y,illumination[i].z, flicker };
			glLightfv(GL_LIGHT5, GL_POSITION, l5Position);
			glLightfv(GL_LIGHT5, GL_DIFFUSE, lDiffuse);
			glLightf(GL_LIGHT5, GL_SPOT_CUTOFF, 30.0);
			glLightf(GL_LIGHT5, GL_SPOT_EXPONENT, 90.0);
			glLightfv(GL_LIGHT5, GL_SPOT_DIRECTION, lDirection);
		}
		else if (i == 5) {
			GLfloat l6Position[] = { illumination[i].x,illumination[i].y,illumination[i].z, flicker };
			glLightfv(GL_LIGHT6, GL_POSITION, l6Position);
			glLightfv(GL_LIGHT6, GL_DIFFUSE, lDiffuse);
			glLightf(GL_LIGHT6, GL_SPOT_CUTOFF, 30.0);
			glLightf(GL_LIGHT6, GL_SPOT_EXPONENT, 90.0);
			glLightfv(GL_LIGHT6, GL_SPOT_DIRECTION, lDirection);
		}
		else if (i == 6) {
			GLfloat l7Position[] = { illumination[i].x,illumination[i].y,illumination[i].z, flicker };
			glLightfv(GL_LIGHT7, GL_POSITION, l7Position);
			glLightfv(GL_LIGHT7, GL_DIFFUSE, lDiffuse);
			glLightf(GL_LIGHT7, GL_SPOT_CUTOFF, 30.0);
			glLightf(GL_LIGHT7, GL_SPOT_EXPONENT, 90.0);
			glLightfv(GL_LIGHT7, GL_SPOT_DIRECTION, lDirection);
		}



	}
	glDisable(GL_TEXTURE_2D);
}
void renderMap()

{


	glColor3f(1, 1, 1);
	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

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
	glDisable(GL_TEXTURE_2D);
}
void renderObsticles()

{

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);	// Bind the ground texture
	int centerx = ((int)round(player.x / 7));
	int centerz = ((int)round(player.z / 7));


	for (int x = -4 + centerx;x - centerx <= 4;x++) {
		for (int z = -4 + centerz;z - centerz <= 4;z++) {
			if ((abs(centerx - x) + abs(centerz - z) > 3)||isBuilding(x*7,z*7))continue;
			if (isNight) {
				int whichMap = ((x + z) % 5 + 5) % 5;
				glPushMatrix();
				glTranslatef(x*7,0,z*7);
				if (whichMap == 1) {
					drawCar(0, -1.5, true);
				}
				else if (whichMap == 2) {
					drawCar(1.5, -1, false);
					drawCar(-2.5, 1, false);
				}
				else if (whichMap == 3) {
					drawCar(-1.5, 0, false);
				}
				else if (whichMap == 4) {
					drawCar(0, -2.5, true);
					drawCar(-1.5, 1, false);
				}
				glPopMatrix();

			}
			else {
				for (int i = -3;i <= 3;i++) {
					for (int j = -3;j <= 3;j++) {
						if (isObsticle(x * 7 + i, z * 7 + j)) {
							glPushMatrix();
							glTranslatef(x * 7 + i, -0.5 + 0.01, z * 7 + j);
							glColor3f(0, 0, 0);
							glutSolidCube(1);
							glPopMatrix();


						}
					}
				}
			}
		}
	}
	glDisable(GL_TEXTURE_2D);
}
void renderGround()

{

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing
	glColor3f(1, 1, 1);
	if (isNight) {
		glBindTexture(GL_TEXTURE_2D, tex_road.texture[0]);	// Bind the ground texture

	}
	else {

		glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);	// Bind the ground texture
	}
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
	glDisable(GL_TEXTURE_2D);
}
void renderEnemy() {
	double dot = player.z - enemy.z;
	double det = player.x - enemy.x;
	double angle = rad2deg(atan2(det, dot));
	glColor3f(1.0, 1.0, 1.0);
	glPushMatrix();
	glTranslated(enemy.x, enemy.y, enemy.z - 0.02);
	glRotatef(angle, 0, 1, 0);
	drawSphere(0.38, 1.48, 2.1, 0.08, tex_eye);
	drawSphere(-0.38, 1.48, 2.1, 0.08, tex_eye);
	glScalef(0.15, 0.15, 0.15);

	model_enemy.Draw();
	glPopMatrix();

}
void renderPlayer() {
	glPushMatrix();
	glColor3f(0.7, 0.7, 0.7);
	glTranslated(player.x, player.y + 0.55, player.z - 0.02);
	glRotatef(-90 + ((movingFront + movingBack + movingRight + movingLeft) ? (movingFront * movingRight * 360 + movingBack * 180 + movingRight * 270 + movingLeft * 90) /
		(movingFront + movingBack + movingRight + movingLeft) : 0), 0, 1, 0);
	glRotatef(angleFront + 168, 0, 1, 0);
	glRotatef(94, 1, 0, 0);
	//glScalef(0.015, 0.015, 0.015);
	//model_player.Draw();
	//glRotatef(100, 1, 0, 1);
	model_character[playerFrame].Draw();

	glPopMatrix();

}
void renderScreen() {
	glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);

	double dot = (target.x - player.x) * front.x + (target.z - player.z) * front.z;
	double det = -(target.x - player.x) * front.z + (target.z - player.z) * front.x;
	double angle = rad2deg(atan2(det, dot));


	glPushMatrix();
	glColor3f(1, 1, 1);
	Vector3f where = eye + (center - eye).unit() * 0.2;
	glTranslatef(where.x, where.y, where.z);
	glRotatef(angleFront + (view == 2 ? 180 : 0), 0, 1, 0);
	glRotatef(-angleUp, 0, 0, 1);
	print(Vector3f(0, 0.04, 0.119), to_string(score));
	if (playerSpeed == 1) {
		if (loadingPower < 8) {
			//print(Vector3f(0, 0.03, 0.1), "loading powerUp " + to_string((int)(8 - loadingPower)));
			glPushMatrix();
			glColor3f(0.24, 0.7, 0.225);
			glBegin(GL_QUADS);
			glVertex3f(0, 0.035, 0.1);
			glVertex3f(0, 0.03, 0.1);
			glVertex3f(0, 0.03, 0.1 + loadingPower / 200);
			glVertex3f(0, 0.035, 0.1 + loadingPower / 200);
			glEnd();
			glPopMatrix();

		}
		else {
			print(Vector3f(0, 0.02, 0.09), "press Q to activate powerUp");
			glPushMatrix();
			glColor3f(0.5, flicker, 1.0);
			glBegin(GL_QUADS);
			glVertex3f(0, 0.035, 0.1);
			glVertex3f(0, 0.03, 0.1);
			glVertex3f(0, 0.03, 0.14);
			glVertex3f(0, 0.035, 0.14);
			glEnd();
			glPopMatrix();

		}
	}
	glColor3f(1, 1, 1);
	drawCircle(Vector3f(0, 0.06, 0.12), 0, 0.015);
	glColor3f(1, 0, 0);
	drawCircle(Vector3f(-0.0001, 0.06, 0.12), 0.01, 0.0125);

	glTranslatef(-0.0001, 0.06, 0.12);
	if (health > 0)
		drawHealth(0, 5);
	if (health > 1)
		drawHealth(-15, -20);
	if (health > 2)
		drawHealth(15, -20);
	glRotated(angle, 1, 0, 0);
	glTranslatef(0, 0.0115, 0);
	glBegin(GL_QUADS);
	glVertex3f(0, -0.003, 0);
	glVertex3f(0, 0, -0.003);
	glVertex3f(0, 0.003, 0);
	glVertex3f(0, 0, 0.003);
	glEnd();
	glPopMatrix();
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
}
void renderEndGameScreen(bool won) {
	glPushMatrix();
	glPushMatrix();
	glColor3f(0.7, 0.7, 0.7);
	glRotatef(angleCoin, 0, 1, 0);
	glTranslatef(0, 0.5, 0);
	glRotatef(94, 1, 0, 0);
	model_character[5].Draw();
	glPopMatrix();
	glPushMatrix();
	glColor3f(1, 1, 1);
	Vector3f where = eye + (center - eye).unit() * 0.2;
	glTranslatef(where.x, where.y, where.z);
	glRotatef(angleFront + (view == 2 ? 180 : 0), 0, 1, 0);
	glRotatef(-angleUp, 0, 0, 1);
	if (won) {
		glColor3f(0.1, 0.9, 0.1);
		print(Vector3f(-0.05, 0.025, 0), "YOU WON ");
		print(Vector3f(-0.05, 0.02, 0), "SCORE");

	}
	else {
		glColor3f(0.9, 0.1, 0.1);
		print(Vector3f(-0.05, 0.025, 0), "YOU LOST");
		print(Vector3f(-0.05, 0.02, 0), "SCORE");
	}
	print(Vector3f(-0.03, 0.019, 0.0), to_string(score));
	glPopMatrix();
	glPopMatrix();
}


void Special(int key, int x, int y) {
	if (!isCompleted) {
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
	}
	else {
		if (key == '8') {
			exit(0);

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
	if (!isCompleted) {

		switch (button)
		{
		case ' ': {
			if ((jump < 2) && (!isFalling)) {
				sndPlaySound(TEXT("sounds/wallCollision.wav"), SND_ASYNC | SND_FILENAME);
				playerV += Vector3f(0, 0.18, 0);
				jump++;
			}
			break;
		}
		case 'q': {
			if (loadingPower >= 8) {
				playerSpeed = 1.5;
				loadingPower = 0;
			}
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

		case 27:
			exit(0);
			break;
		default:
			break;
		}
	}
	else {
		if (button == 27) {
			exit(0);

		}
		if (button == 'r') {
			startGame();
		}
	}



	glLoadIdentity();

	glutPostRedisplay();
	gluLookAt(eye.x, eye.y, eye.z, center.x, center.y, center.z, up.x, up.y, up.z);



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

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (!isCompleted) {
		ShowCursor(false);
		InitLightSource();
		InitMaterial();
		//rendering
		renderTarget();
		drawSun();
		renderGround();// Draw Ground
		renderObsticles();
		renderCoins();
		renderScreen();
		renderMap();
		if (isNight) {
			renderLightPosts();
		}
		renderEnemy();
		if (view != 1)
			renderPlayer();

		glPushMatrix();
		GLUquadricObj* qobj;
		qobj = gluNewQuadric();
		glTranslated(player.x, 0, player.z);
		glColor3f(0.7 + skyDim, 0.7 + skyDim, 0.7 + skyDim);
		glRotated(90, 1, 0, 1);
		if (isNight) {
			glBindTexture(GL_TEXTURE_2D, tex_sky_night);

		}
		else {
			glBindTexture(GL_TEXTURE_2D, tex_sky);

		}
		gluQuadricTexture(qobj, true);
		gluQuadricNormals(qobj, GL_SMOOTH);
		gluSphere(qobj, 200, 100, 100);
		gluDeleteQuadric(qobj);
		glPopMatrix();


	}
	else {
		renderEndGameScreen(wonEndGame);
	}
	glutSwapBuffers();
}
void tick(int value) {
	move();
	toFlicker--;
	if (toFlicker <= 0) {
		toFlicker = 50;
		flicker = !flicker;
	}
	if (!isCompleted) {

		moveEnemy();
	}
	//power up speed
	if (playerSpeed == 1.5) {
		speedCount--;
		if (speedCount == 0) {
			speedCount = 300;
			playerSpeed = 1;
		}
	}
	if (target.dis(player) < 2) {
		if (!isNight) {
			sndPlaySound("sounds/bottleDrink.wav", SND_ASYNC | SND_FILENAME);

		}
		restart(true);
	}
	if ((enemy.dis(player) < 2) || (player.y < -30)) {
		restart(false);

	}
	if (enemy.dis(player) < 4) {
		sndPlaySound(TEXT("sounds/monster.wav"), SND_ASYNC | SND_FILENAME);

	}
	if (!isNight) {
		if (sunDim >= -0.6) {

			sunDim -= 0.0005;
		}
		if (skyDim >= -0.5) {
			skyDim -= 0.0001;
		}
		if (light1 >= -0.8) {
			light1 -= 0.0001;
		}
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
	//model_can.Load("models/cola/Pot Cola N260411.3ds");
	model_drink.Load("models/drink/drink.3ds");
	model_lightPost.Load("models/lightPost/lightpost.3ds");
	model_car.Load("models/ambulance/Car Ambulance N230610.3ds");
	model_target.Load("models/cola/bottle.3ds");
	model_building1.Load("Models/building1/Tower Constantino Eleninskaya Kremlin N120615.3DS");
	model_coin.Load("Models/gold/gold.3ds");
	tex_ground.Load("Textures/street.bmp");
	tex_sun.Load("Textures/sun.bmp");
	tex_moon.Load("Textures/moon.bmp");
	tex_road.Load("Textures/esfalt.bmp");
	//player
	Model_3DS c_6;
	c_6.Load("Models/spartan/-6.3DS");
	Model_3DS c_5;
	c_5.Load("Models/spartan/-5.3DS");
	Model_3DS c_4;
	c_4.Load("Models/spartan/-4.3DS");
	Model_3DS c_3;
	c_3.Load("Models/spartan/-3.3DS");
	Model_3DS c_2;
	c_2.Load("Models/spartan/-2.3DS");
	Model_3DS c_1;
	c_1.Load("Models/spartan/-1.3DS");
	Model_3DS c1;
	c1.Load("Models/spartan/1.3DS");
	Model_3DS c2;
	c2.Load("Models/spartan/2.3DS");
	Model_3DS c3;
	c3.Load("Models/spartan/3.3DS");
	Model_3DS c4;
	c4.Load("Models/spartan/4.3DS");
	Model_3DS c5;
	c5.Load("Models/spartan/5.3DS");
	model_character.push_back(c_6);
	model_character.push_back(c_5);
	model_character.push_back(c_4);
	model_character.push_back(c_3);
	model_character.push_back(c_2);
	model_character.push_back(c_1);
	model_character.push_back(c1);
	model_character.push_back(c2);
	model_character.push_back(c3);
	model_character.push_back(c4);
	loadBMP(&tex_eye, "Models/enemy/eye.bmp", true);
	loadBMP(&tex_sky, "Textures/blu-sky-3.bmp", true);
	loadBMP(&tex_sky_night, "Textures/night.bmp", true);

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
	sndPlaySound(TEXT("sounds/monster.wav"), SND_ASYNC | SND_FILENAME);
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