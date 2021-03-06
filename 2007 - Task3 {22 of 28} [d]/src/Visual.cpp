#include <gl/glut.h>
#include <gl/glext.h>
#include <iostream>
#include <windows.h>
#include <string>
#include <cmath>
#include <utility>
#include "Visual.h"
#include "loadbmp.h"
#include "Model.h"

#define SENSE 0.01f
#define PI 3.1415926535897f

#define FULLSCREEN

using std::string;
using std::vector;
using std::cout;
using std::pair;
using std::endl;
using std::runtime_error;
using std::make_pair;

GLuint wallTex, floorTex, blackTex, arrowTex, stoneTex; // textures

int wndH, wndW; // window size
float mouseX, mouseY; // mouse angles in radians

unsigned long lastTickCount;
float time, dTime;

bool forward, backward, left, right; // moving?

float playerSpeed;

Vector3d player; // player position

GLUquadricObj *gluSphereQuadric, *gluStoneQuadric; // sphere for  light

// the one who's walking in the labyrinth
Vector3d guidePos;
Vector3d guideDir;
float guideSpeed;
int wayPos;
bool hasGuide;
bool guided;

unsigned int fixedPos;

Model *mazeModel, *floorModel, *ceilModel; // maze model

Maze* maze; // maze

vector<Point2d>* mazeWay;

vector<pair<Vector3d, float> > stones;

GLuint LoadTexture(char* fileName, bool repeatS, bool repeatT) 
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	IMAGE img;
	if (LoadBMP(fileName, &img) == 0) 
	{
		cout << "Could not read texture file \"" << fileName << "\"" << endl;
		throw runtime_error("");
	}

	gluBuild2DMipmaps(GL_TEXTURE_2D,   
		3,                     // texture format
		img.width, img.height, // size
		GL_RGB,                // data format
		GL_UNSIGNED_BYTE,      // data type
		img.data);             // data pointer
	// duh, we can't free it

	// Set texture params
	if (repeatS)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	if (repeatT)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	return texture;
}

void LoadTextures() 
{
	wallTex  = LoadTexture("textures/bricks.bmp", true, false);
	blackTex = LoadTexture("textures/black.bmp", true, true);
	floorTex = LoadTexture("textures/floor.bmp", true, true);
	arrowTex = LoadTexture("textures/arrow.bmp", true, true);
	stoneTex = LoadTexture("textures/stone.bmp", true, true);
}

void InitGl() 
{
	// enables...
	glEnable(GL_DEPTH_TEST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_NORMALIZE);

	// material
	GLfloat EmissiveM[4] = { 0.2f, 0.2f, 0.2f, 0.0f };
	GLfloat DiffusiveM[4] = { 0.5f, 0.5f, 0.5f, 0.7f };
	GLfloat SpecularM[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	GLfloat AmbientM[4] = { 0.3f, 0.3f, 0.3f, 0.0f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, EmissiveM);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, DiffusiveM);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, SpecularM);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, AmbientM);
	//glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 10);

	// light
	GLfloat DiffuseL[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat SpecularL[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat AmbientL[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseL);
	glLightfv(GL_LIGHT0, GL_SPECULAR, SpecularL);
	glLightfv(GL_LIGHT0, GL_AMBIENT, AmbientL);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.1);

	// light on
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glShadeModel(GL_SMOOTH);

	// blendfunc
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void AddSquare(ModelPart* target, int xp, int yp, Vector3d a, Vector3d dx, Vector3d dy, Vector3d normal) 
{
	for (int i = 0; i < xp; i++) 
	{
		for (int j = 0; j < yp; j++) 
		{
			Vector3d caa = Vector3d((float) i / xp, (float) j / yp, 0);
			Vector3d cab = Vector3d((float) i / xp, (float) (j + 1) / yp, 0);
			Vector3d cba = Vector3d((float) (i + 1) / xp, (float) j / yp, 0);
			Vector3d cbb = Vector3d((float) (i + 1) / xp, (float) (j + 1) / yp, 0);

			Vector3d aa = a + dx * caa.x + dy * caa.y;
			Vector3d ab = a + dx * cab.x + dy * cab.y;
			Vector3d ba = a + dx * cba.x + dy * cba.y;
			Vector3d bb = a + dx * cbb.x + dy * cbb.y;

			target->add(aa, normal, caa);
			target->add(ab, normal, cab);
			target->add(bb, normal, cbb);
			target->add(aa, normal, caa);
			target->add(ba, normal, cba);
			target->add(bb, normal, cbb);
		}
	}
}

void GenerateMazeModel(Maze* maze) 
{
	// floor
	ModelPart* floor = new ModelPart(floorTex, maze->getX() * maze->getY() * 3 * 2 * 3 * 3);
	for (int x = 0; x < maze->getX(); x++)
		for (int y = 0; y < maze->getY(); y++)
			if (maze->at(x, y) == 0)
				AddSquare(floor, 3, 3, Vector3d(x, y, 0), Vector3d(1, 0, 0), Vector3d(0, 1, 0), Vector3d(0, 0, 1));

	// black
	ModelPart* ceil = new ModelPart(blackTex, maze->getX() * maze->getY() * 3 * 2);
	for (int x = 0; x < maze->getX(); x++)
		for (int y = 0; y < maze->getY(); y++)
			if (maze->at(x, y) == 1)
				AddSquare(ceil, 1, 1, Vector3d(x, y, 1), Vector3d(1, 0, 0), Vector3d(0, 1, 0), Vector3d(0, 0, 1));

	// walls
	ModelPart* wall = new ModelPart(wallTex, maze->getX() * maze->getY() * 3 * 2 * 4 * 3 * 3);
	for (int x = 0; x < maze->getX(); x++) 
	{
		for (int y = 0; y < maze->getY(); y++) 
		{
			if (maze->at(x, y) == 1 && maze->at(x + 1, y) == 0)
				AddSquare(wall, 3, 3, Vector3d(x + 1, y, 0), Vector3d(0, 1, 0), Vector3d(0, 0, 1), Vector3d(1, 0, 0));
			if (maze->at(x, y) == 1 && maze->at(x - 1, y) == 0)
				AddSquare(wall, 3, 3, Vector3d(x, y, 0), Vector3d(0, 1, 0), Vector3d(0, 0, 1), Vector3d(-1, 0, 0));
			if (maze->at(x, y) == 1 && maze->at(x, y + 1) == 0)
				AddSquare(wall, 3, 3, Vector3d(x, y + 1, 0), Vector3d(1, 0, 0), Vector3d(0, 0, 1), Vector3d(0, 1, 0));
			if (maze->at(x, y) == 1 && maze->at(x, y - 1) == 0)
				AddSquare(wall, 3, 3, Vector3d(x, y, 0), Vector3d(1, 0, 0), Vector3d(0, 0, 1), Vector3d(0, -1, 0));
		}
	}

	mazeModel = new Model(); 
	floorModel = new Model(); 
	ceilModel = new Model(); 
	mazeModel->add(wall);
	ceilModel->add(ceil);
	floorModel->add(floor);
}

void MovePlayer() 
{
	if (hasGuide && guided) 
	{
		player = guidePos;
	} 
	else 
	{
		if (forward) 
		{
			player.x += sin(mouseX) * cos(mouseY) * dTime * playerSpeed;
			player.y += cos(mouseX) * cos(mouseY) * dTime * playerSpeed;
			player.z += sin(mouseY) * dTime * playerSpeed;
		}
		if (backward) 
		{
			player.x -= sin(mouseX) * cos(mouseY) * dTime * playerSpeed;
			player.y -= cos(mouseX) * cos(mouseY) * dTime * playerSpeed;
			player.z -= sin(mouseY) * dTime * playerSpeed;
		}
		if (left) 
		{
			player.x += -cos(mouseX) *  dTime * playerSpeed;
			player.y += sin(mouseX) *  dTime * playerSpeed;
		}
		if (right) 
		{
			player.x -= -cos(mouseX) *  dTime * playerSpeed;
			player.y -= sin(mouseX) *  dTime * playerSpeed;
		}
	}

	if (player.x < 0)
		player.x = 0;
	if (player.y < 0)
		player.y = 0;
	if (player.x > maze->getX())
		player.x = maze->getX();
	if (player.y > maze->getY())
		player.y = maze->getY();
	if (player.z < 0.5)
		player.z = 0.5;

	Vector3d probe;
	if (player.z < 1.0) 
	{
		probe = player + Vector3d(0.2, 0, 0);
		if (maze->at(probe.x, probe.y) == 1) 
			player.x = ceil(player.x) - 0.2;
		probe = player - Vector3d(0.2, 0, 0);
		if (maze->at(probe.x, probe.y) == 1) 
			player.x = floor(player.x) + 0.2;
		probe = player + Vector3d(0, 0.2, 0);
		if (maze->at(probe.x, probe.y) == 1) 
			player.y = ceil(player.y) - 0.2;
		probe = player - Vector3d(0, 0.2, 0);
		if (maze->at(probe.x, probe.y) == 1) 
			player.y = floor(player.y) + 0.2;
	} 
	else if  (player.z < 1.2)
		if (maze->at(player.x, player.y) == 1) 
			player.z = 1.2;
}

void DrawStone(int i) 
{
	glPushMatrix();
	glTranslatef(stones[i].first.x, stones[i].first.y, stones[i].first.z);
	gluSphere(gluStoneQuadric, stones[i].second, 16, 16);
	glPopMatrix();
}

void DrawStones() 
{
	glBindTexture(GL_TEXTURE_2D, stoneTex);
	for (unsigned int i = 0; i < stones.size(); i++)
		DrawStone(i);
}

void GenStones() 
{
	Matrix<bool> way(maze->getX(), maze->getY());
	way.fill(false);
	if (hasGuide)
		for (unsigned int i = 0; i < mazeWay->size(); i++)
			way[mazeWay->at(i).x][mazeWay->at(i).y] = true;

	for (int i = 0; i < max(maze->getX() * maze->getY() / 25, maze->getX() + maze->getY()); i++) 
	{
		int x = maze->getX() * rand() / (RAND_MAX + 1);
		int y = maze->getY() * rand() / (RAND_MAX + 1);
		if (maze->at(Point2d(x, y)) == 0 && !way[x][y]) 
		{
			float r = 0.01 + 0.16 * rand() / (RAND_MAX + 1);
			stones.push_back(make_pair(Vector3d(x + 0.2f + 0.6f * rand() / RAND_MAX, y + 0.2f + 0.6f * rand() / RAND_MAX, r), r));
			way[x][y] = true;
		}
	}
}

void GotoFixedInMaze(int n, int maxn) 
{
	int index = (mazeWay->size() - 2) * n / maxn;
	player.x = mazeWay->at(index).x + 0.5;
	player.y = mazeWay->at(index).y + 0.5;
	player.z = 0.5;
	mouseX = atan2((float) mazeWay->at(index + 1).x - mazeWay->at(index).x, (float) mazeWay->at(index + 1).y - mazeWay->at(index).y);
	mouseY = 0;
	fixedPos = n;
}

void GotoFixedAboveMaze(int n, int maxn) 
{
	player.x = 0.5 + (maze->getX() - 1) * (0.5 + 0.5 * cos(2 * PI * n / maxn));
	player.y = 0.5 + (maze->getY() - 1) * (0.5 + 0.5 * sin(2 * PI * n / maxn));
	player.z = (maze->getX() + maze->getY()) / 6;
	mouseX = 3*PI/2 - 2 * PI * n / maxn;
	mouseY = -PI / 4;
}

void GotoFixedPos(int n) 
{
	if (hasGuide) 
	{
		if (n < 5)
			GotoFixedInMaze(n, 5);
		else
			GotoFixedAboveMaze(n - 5, 5);
	} 
	else
		GotoFixedAboveMaze(n, 10);
}

void GotoNextFixedPos() 
{
	fixedPos++;
	fixedPos %= 10;
	GotoFixedPos(fixedPos);
}


void InitGuide() 
{
	guidePos = Vector3d(0.5f, 0.5f, 0.5f);
	guideDir = Vector3d(mazeWay->at(1).x, mazeWay->at(1).y, 0.0f);
	wayPos = 1;
}

void MoveGuide() 
{
	Vector3d target0(mazeWay->at(wayPos).x + 0.5, mazeWay->at(wayPos).y + 0.5, 0.5);
	Vector3d target1(mazeWay->at(wayPos + 1).x + 0.5, mazeWay->at(wayPos + 1).y + 0.5, 0.5);

	float t0m = max(1 - (target0 - guidePos).abs2() / 4, 0.1);
	float t1m = max(1 - (target1 - guidePos).abs2() / 4, 0.1);

	Vector3d d = ((target0 - guidePos).normalize() * t0m + (target1 - guidePos).normalize() * t1m).normalize();
	float vcos = d * guideDir;
	Vector3d turn = (d * (1 / vcos) - guideDir);
	if (turn.abs2() > (1.1f * dTime) * (1.1f * dTime))
		turn = turn.normalize() * (1.1f * dTime);
	guideDir = (guideDir + turn).normalize();
	guidePos = guidePos + guideDir * guideSpeed * dTime;

	if ((guidePos - target0).abs2() < 0.5)
		wayPos++;
	if (wayPos == mazeWay->size() - 1)
		InitGuide();
}

void DrawArrow(int i) 
{
	glPushMatrix();
	glTranslatef(mazeWay->at(i).x + 0.5, mazeWay->at(i).y + 0.5, 0.1 + 0.05 * sin(0.3 * time * (mazeWay->at(i).x % 5 + mazeWay->at(i).y % 7)));
	glRotatef(- atan2((float) mazeWay->at(i + 1).x - mazeWay->at(i).x, (float) mazeWay->at(i + 1).y - mazeWay->at(i).y) * 180.0f / PI, 0, 0, 1);

	float dSizeX = 1.0f + 0.05 * (1 + mazeWay->at(i).x % 3) * sin(1.8 * time * (mazeWay->at(i).x % 7 + mazeWay->at(i).y % 3));
	float dSizeY = 1.0f + 0.05 * (1 + mazeWay->at(i).y % 3) * cos(1.8 * time * (mazeWay->at(i).x % 5 + mazeWay->at(i).y % 5));
	glScalef(dSizeX, dSizeY, 1);

	glBegin(GL_TRIANGLES);
	glNormal3f(0, 0, 1); 
	glTexCoord2f(1,   0); glVertex3f( 0.1f,-0.1f, 0.05f);
	glTexCoord2f(0,   0); glVertex3f(-0.1f,-0.1f, 0.05f);
	glTexCoord2f(0.5, 1); glVertex3f( 0.0f, 0.2f, 0.05f);

	glNormal3f(0, 0, -1); 
	glTexCoord2f(1,   0); glVertex3f( 0.1f,-0.1f,  0);
	glTexCoord2f(0,   0); glVertex3f(-0.1f,-0.1f,  0);
	glTexCoord2f(0.5, 1); glVertex3f( 0.0f, 0.2f,  0);
	glEnd();

	glBegin(GL_QUADS);
	glNormal3f(0, -1, 0); 
	glTexCoord2f(1,   0); glVertex3f( 0.1f,-0.1f,  0.05f);
	glTexCoord2f(1,   0); glVertex3f( 0.1f,-0.1f,  0);
	glTexCoord2f(0,   0); glVertex3f(-0.1f,-0.1f,  0);
	glTexCoord2f(0,   0); glVertex3f(-0.1f,-0.1f,  0.05f);

	glNormal3f(-3, 1, 0); 
	glTexCoord2f(0,   0); glVertex3f(-0.1f,-0.1f,  0.05f);
	glTexCoord2f(0,   0); glVertex3f(-0.1f,-0.1f,  0);
	glTexCoord2f(0.5, 1); glVertex3f( 0.0f, 0.2f,  0);
	glTexCoord2f(0.5, 1); glVertex3f( 0.0f, 0.2f,  0.05f);

	glNormal3f( 3, 1, 0); 
	glTexCoord2f(1,   0); glVertex3f( 0.1f,-0.1f,  0.05f);
	glTexCoord2f(1,   0); glVertex3f( 0.1f,-0.1f,  0);
	glTexCoord2f(0.5, 1); glVertex3f( 0.0f, 0.2f,  0);
	glTexCoord2f(0.5, 1); glVertex3f( 0.0f, 0.2f,  0.05f);
	glEnd();

	glPopMatrix();
}

void DrawArrows() 
{
	glBindTexture(GL_TEXTURE_2D, arrowTex);
	for (unsigned int i = 0; i < mazeWay->size() - 1; i++)
		DrawArrow(i);
}

void DrawLightBulb() 
{
	if (!(hasGuide && guided)) 
	{
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
		glColor4f(1, 1, 1, 1);
		glPushMatrix();
		glTranslatef(guidePos.x, guidePos.y, guidePos.z);
		gluSphere(gluSphereQuadric, 0.03, 4, 4);
		glPopMatrix();
		glEnable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);
	}
}

void OnDisplay() 
{
	// time...
	// Sleep(10);
	dTime = (GetTickCount() - lastTickCount) / 1000.0f;
	time += dTime;
	lastTickCount = GetTickCount();

	// move guide
	if (hasGuide) 
		MoveGuide();

	// move player
	MovePlayer();

	// clear
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// place player
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(75.0, ((float) wndW) / wndH, 0.1, 160);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (hasGuide && guided)
		gluLookAt(player.x, player.y, player.z, player.x + guideDir.x, player.y + guideDir.y, player.z + sin(mouseY), 0, 0, 1);
	else
		gluLookAt(player.x, player.y, player.z, player.x + sin(mouseX) * cos(mouseY), player.y + cos(mouseX) * cos(mouseY), player.z + sin(mouseY), 0, 0, 1);

	// place light
	if (!hasGuide) 
	{
		guidePos = player;
		if (guidePos.z < 0.5)
			guidePos.z = 0.5;
	}
	GLfloat l_position[4] = {guidePos.x, guidePos.y, guidePos.z, 1};
	glLightfv(GL_LIGHT0, GL_POSITION, l_position);

	// Floor - stencil mask
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 1, 1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glBegin(GL_QUADS);
	glVertex3f(0, 0, 0);
	glVertex3f(maze->getX(), 0, 0);
	glVertex3f(maze->getX(), maze->getY(), 0);
	glVertex3f(0, maze->getY(), 0);
	glEnd();
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

	// reflection
	glStencilFunc(GL_EQUAL,1,1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glPushMatrix();
	glScalef(1,1,-1);
	glLightfv(GL_LIGHT0, GL_POSITION, l_position);
	mazeModel->draw();
	if (hasGuide)
		DrawArrows();
	DrawLightBulb();
	DrawStones();
	glPopMatrix();
	glDisable(GL_STENCIL_TEST);

	// place light back
	glLightfv(GL_LIGHT0, GL_POSITION, l_position);

	// floor
	glEnable(GL_BLEND);
	floorModel->draw();
	glDisable(GL_BLEND);

	// upper world
	mazeModel->draw();
	ceilModel->draw();
	if (hasGuide)
		DrawArrows();
	DrawLightBulb();
	DrawStones();

	// shadows
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(-3, -3);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 1, 1);
	glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glPushMatrix();
	GLfloat Projection[16] = {guidePos.z, 0, 0, 0,   0, guidePos.z, 0, 0,   -guidePos.x, -guidePos.y, 0, -1,   0, 0, 0, guidePos.z};
	glMultMatrixf(Projection);
	for (unsigned int i = 0; i < stones.size(); i++) 
	{
		float dist = (Vector3d(stones[i].first.x, stones[i].first.y, 0.5) - guidePos).abs2();
		if (dist > 2)
			continue;
		glColor4f(0, 0, 0, (2 - dist) * 0.15);
		DrawStone(i);
	}
	if (hasGuide) 
	{
		for (unsigned int i = 0; i < mazeWay->size() - 1; i++) 
		{
			float dist = (Vector3d(mazeWay->at(i).x + 0.5, mazeWay->at(i).y + 0.5, 0.5) - guidePos).abs2();
			if (dist > 2)
				continue;
			glColor4f(0, 0, 0, (2 - dist) * 0.15);
			DrawArrow(i);
		}
	}
	glPopMatrix();
	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING); 
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_POLYGON_OFFSET_FILL);

	glutSwapBuffers();
}

void OnReshape(int x, int y) 
{
	wndW = x;
	wndH = y;
	glViewport(0, 0, wndW, wndH);
}

void OnIdle() 
{
	glutPostRedisplay();
}

void OnKeyDown(unsigned char key, int x, int y) 
{
	if (key == 27)
		exit(0);
	if (key == 'w')
		forward = true;
	if (key == 's')
		backward = true;
	if (key == 'a')
		left = true;
	if (key == 'd')
		right = true;
	if (key >= '0' && key <= '9')
		GotoFixedPos(key - '0');
	if (key == ' ')
		GotoNextFixedPos();
}

void OnKeyUp(unsigned char key, int x, int y) 
{
	if (key == 'w')
		forward = false;
	if (key == 's')
		backward = false;
	if (key == 'a')
		left = false;
	if (key == 'd')
		right = false;
	if (key == 13)
		guided = !guided;
}

void OnMouseMove(int x, int y) 
{
	if (!(x == wndW / 2 && y == wndH / 2)) 
	{
		mouseX += (x - wndW / 2) * SENSE;
		mouseY += (wndH / 2 - y) * SENSE;

		if (mouseY >  PI * 0.45f) mouseY =  PI * 0.45f; 
		if (mouseY < -PI * 0.45f) mouseY = -PI * 0.45f; 

		glutWarpPointer(wndW / 2, wndH / 2);
	}
}

void Visualize(Maze* maze, vector<Point2d>* way) 
{
	srand(GetTickCount());

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB | GLUT_ALPHA | GLUT_STENCIL);

	glutInitWindowSize(640, 480);
	glutCreateWindow("asgn3");

	InitGl();
	LoadTextures();

	glutIdleFunc(OnIdle);
	glutPassiveMotionFunc(OnMouseMove); 
	glutDisplayFunc(OnDisplay);
	glutKeyboardFunc(OnKeyDown);
	glutKeyboardUpFunc(OnKeyUp);
	glutReshapeFunc(OnReshape);

	glutSetCursor(GLUT_CURSOR_NONE);

#ifdef FULLSCREEN
	DEVMODE devMode;        
	memset(&devMode, 0, sizeof(devMode));
	devMode.dmSize = sizeof(devMode);  
	devMode.dmBitsPerPel = 32;
	devMode.dmPelsWidth = 640;    
	devMode.dmPelsHeight = 480;    
	devMode.dmFields = DM_PELSWIDTH | DM_BITSPERPEL | DM_PELSHEIGHT;
	ChangeDisplaySettings(&devMode, CDS_FULLSCREEN);
	glutFullScreen();  
#endif
	time = 0.0f;
	lastTickCount = GetTickCount();

	player = Vector3d(0.0f, 0.0f, 0.0f);

	mazeWay = way;

	forward = false;
	backward = false;
	left = false;
	right = false;

	playerSpeed = 3.0f;

	GenerateMazeModel(maze);
	::maze = maze;

	if (way->size() > 0) 
	{
		hasGuide = true;
		InitGuide();
		guideSpeed = 1.0f;
		way->push_back(way->back());
		guided = false;
	} 
	else
		hasGuide = false;

	GenStones();

	fixedPos = 0;

	gluSphereQuadric = gluNewQuadric();
	gluStoneQuadric = gluNewQuadric();
	gluQuadricTexture(gluStoneQuadric, GL_TRUE);

	glutMainLoop();
}