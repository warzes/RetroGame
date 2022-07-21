#include "stdafx.h"
#include "Game.h"
#include "GameData.h"
#include "Engine.h"


#define numSect 4
#define numWall 16

struct Time
{
	int fr1, fr2;           //frame 1 frame 2, to create constant frame rate
}; 
Time T;
struct keys
{
	int w, s, a, d;           //move up, down, left, right
	int sl, sr;             //strafe left, right 
	int m;                 //move up, down, look up, down
}; 
keys K;

struct math
{
	float cos[360];
	float sin[360];
};
math M;

struct player
{
	int x, y, z;
	int a; // player angle of rotation left right
	int l; // variable to look up and down
};
player P;

struct walls
{
	int x1, y1; // bottom line point 1
	int x2, y2; // bottom line point 2
	int c; // wall color
};
walls W[30];

struct sectors
{
	int ws, we; // wall number start and end
	int z1, z2; // height of bottom and top
	int d; // add y distances to sort drawing order
	int c1, c2; // bottom and top color
	int surf[VIEWPORT_X];//to hold points for surfaces
	int surface;//is there a surfaces to draw
};

sectors S[30];

void pixel(int x, int y, int c)                  //draw a pixel at x/y with rgb
{
	int rgb[3];
	if (c == 0) { rgb[0] = 255; rgb[1] = 255; rgb[2] = 0; } //Yellow	
	if (c == 1) { rgb[0] = 160; rgb[1] = 160; rgb[2] = 0; } //Yellow darker	
	if (c == 2) { rgb[0] = 0; rgb[1] = 255; rgb[2] = 0; } //Green	
	if (c == 3) { rgb[0] = 0; rgb[1] = 160; rgb[2] = 0; } //Green darker	
	if (c == 4) { rgb[0] = 0; rgb[1] = 255; rgb[2] = 255; } //Cyan	
	if (c == 5) { rgb[0] = 0; rgb[1] = 160; rgb[2] = 160; } //Cyan darker
	if (c == 6) { rgb[0] = 160; rgb[1] = 100; rgb[2] = 0; } //brown	
	if (c == 7) { rgb[0] = 110; rgb[1] = 50; rgb[2] = 0; } //brown darker
	if (c == 8) { rgb[0] = 0; rgb[1] = 60; rgb[2] = 130; } //background 
	
	gPixelBuffer->SetPixel(x, y, rgb[0], rgb[1], rgb[2]);
}
void movePlayer()
{
	//move up, down, left, right
	if (gInput.IsKeyDown(GameKey::A) && !gInput.IsKeyDown(GameKey::M)) { P.a -= 4; if (P.a < 0) P.a += 360; }
	if (gInput.IsKeyDown(GameKey::D) && !gInput.IsKeyDown(GameKey::M)) { P.a += 4; if (P.a > 359) P.a -= 360; }
	int dx = M.sin[P.a] * 10.0;
	int dy = M.cos[P.a] * 10.0;
	if (gInput.IsKeyDown(GameKey::W) && !gInput.IsKeyDown(GameKey::M)) { P.x += dx; P.y += dy; }
	if (gInput.IsKeyDown(GameKey::S) && !gInput.IsKeyDown(GameKey::M)) { P.x -= dx; P.y -= dy; }

	//strafe left, right
	if (gInput.IsKeyDown(GameKey::E)) { P.x += dy; P.y -= dx; }
	if (gInput.IsKeyDown(GameKey::Q)) { P.x -= dy; P.y += dx; }

	//move up, down, look up, look down
	if (gInput.IsKeyDown(GameKey::A) && gInput.IsKeyDown(GameKey::M)) { P.l -= 1; }
	if (gInput.IsKeyDown(GameKey::D) && gInput.IsKeyDown(GameKey::M)) { P.l += 1; }
	if (gInput.IsKeyDown(GameKey::W) && gInput.IsKeyDown(GameKey::M)) { P.z -= 4; }
	if (gInput.IsKeyDown(GameKey::S) && gInput.IsKeyDown(GameKey::M)) { P.z += 4; }
}
void clearBackground()
{
	int x, y;
	for (y = 0; y < VIEWPORT_Y; y++)
	{
		for (x = 0; x < VIEWPORT_X; x++) { pixel(x, y, 8); } //clear background color
	}
}

void clipBehindPlayer(int* x1, int* y1, int* z1, int x2, int y2, int z2) // clip line
{
	float da = *y1; // distance plane -> point a
	float db = y2; // distance plane -> point b
	float d = da - db; if (d == 0) d = 1;
	float s = da / (da - db); // intersection factor (between 0 and 1)
	*x1 = *x1 + s * (x2 - (*x1));
	*y1 = *y1 + s * (y2 - (*y1)); if (*y1 == 0) *y1 = 0; // prevent divide by zero
	*z1 = *z1 + s * (z2 - (*z1));
}

void drawWall(int x1, int x2, int b1, int b2, int t1, int t2, int c, int s)
{
	int x, y;
	// Hold difference in distance
	int dyb = b2 - b1; // y distance of bottom line
	int dyt = t2 - t1; // y distance of top line
	int dx = x2 - x1; if (dx == 0) dx = 1;// x distance;
	int xs = x1;
	// CLIP X (один пиксель не рисуется, потом поставить 0)
	if (x1 < 1) x1 = 1; // clip left;
	if (x2 < 1) x2 = 1; // clip left
	if (x1 > VIEWPORT_X - 1) x1 = VIEWPORT_X - 1;// clip right
	if (x2 > VIEWPORT_X - 1) x2 = VIEWPORT_X - 1;// clip right
	// draw x verticle lines
	for (x = x1; x < x2; x++)
	{
		// the y start and end point
		int y1 = dyb * (x - xs + 0.5) / dx + b1; // y bottom point
		int y2 = dyt * (x - xs + 0.5) / dx + t1; // y bottom point

		// CLIP Y (один пиксель не рисуется, потом поставить 0)
		if (y1 < 1) y1 = 1;
		if (y2 < 1) y2 = 1;
		if (y1 > VIEWPORT_Y - 1) y1 = VIEWPORT_Y - 1;
		if (y2 > VIEWPORT_Y - 1) y2 = VIEWPORT_Y - 1;

		// surface
		if (S[s].surface == 1) { S[s].surf[x] = y1; continue; } // save bottom points
		if (S[s].surface == 2) { S[s].surf[x] = y2; continue; } // save top points
		if (S[s].surface == -1) { for (y = S[s].surf[x]; y < y1; y++) { pixel(x, y, S[s].c2); }; } // bottom
		if (S[s].surface == -2) { for (y = y1; y < S[s].surf[x]; y++) { pixel(x, y, S[s].c2); }; } // top

		for (y = y1; y < y2; y++)
		{
			pixel(x, y, c);
		}
	}
}

int dist(int x1, int y1, int x2, int y2)
{
	int distance = sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
	return distance;
}

void draw3D()
{
	int s, w;
	int wx[4], wy[4], wz[4];
	float CS = M.cos[P.a];
	float SN = M.sin[P.a];

	// order sectors by distance
	for (s = 0; s < numSect - 1; s++)
	{
		for (w = 0; w < numSect -s- 1; w++)
		{
			if (S[w].d < S[w + 1].d)
			{
				sectors st = S[w]; S[w] = S[w + 1]; S[w + 1] = st;// пузырьковая сортировка
			}
		}
	}

	// draw sectors
	for(s=0; s <numSect; s++)
	{
		S[s].d = 0; // clear distance
		if (P.z < S[s].z1) { S[s].surface = 1; }
		else if (P.z > S[s].z2) { S[s].surface = 2; }
		else { S[s].surface = 0; }

		for (int loop = 0; loop < 2; loop++) // отрисовка с двух сторон
		{
			for (w = S[s].ws; w < S[s].we; w++)
			{
				//offset bottom 2 points by player
				int x1 = W[w].x1 - P.x, y1 = W[w].y1 - P.y;
				int x2 = W[w].x2 - P.x, y2 = W[w].y2 - P.y;
				// swap for surface
				if (loop == 0) { int swp = x1; x1 = x2; x2 = swp; swp = y1; y1 = y2; y2 = swp; }

				// world X position
				wx[0] = x1 * CS - y1 * SN;
				wx[1] = x2 * CS - y2 * SN;
				wx[2] = wx[0];// top line has the same x
				wx[3] = wx[1];

				// world Y position (depth)
				wy[0] = y1 * CS + x1 * SN;
				wy[1] = y2 * CS + x2 * SN;
				wy[2] = wy[0];// top line has the same y
				wy[3] = wy[1];
				S[s].d += dist(0, 0, (wx[0] + wx[1]) / 2, (wy[0] + wy[1]) / 2); // store this wall distance

				// world Z height
				wz[0] = S[s].z1 - P.z + ((P.l * wy[0]) / 32.0);
				wz[1] = S[s].z1 - P.z + ((P.l * wy[1]) / 32.0);
				wz[2] = wz[0] + S[s].z2; // top line has new z
				wz[3] = wz[1] + S[s].z2;

				// dont draw if behind player
				if (wy[0] < 1 && wy[1] < 1) continue; // wall behind player, dont draw
				// point 1 behind player, clip
				if (wy[0] < 1)
				{
					clipBehindPlayer(&wx[0], &wy[0], &wz[0], wx[1], wy[1], wz[1]);// bottom line
					clipBehindPlayer(&wx[2], &wy[2], &wz[2], wx[3], wy[3], wz[3]);// bottom line
				}
				// point 2 behind player, clip
				if (wy[1] < 1)
				{
					clipBehindPlayer(&wx[1], &wy[1], &wz[1], wx[0], wy[0], wz[0]);// bottom line
					clipBehindPlayer(&wx[3], &wy[3], &wz[3], wx[2], wy[2], wz[2]);// bottom line
				}

				// screen x, screen y position
				if (wy[0] == 0) wy[0] = 1; // я вставил, потому что деление на ноль
				if (wy[1] == 0) wy[1] = 1; // я вставил, потому что деление на ноль
				if (wy[2] == 0) wy[2] = 1; // я вставил, потому что деление на ноль
				if (wy[3] == 0) wy[3] = 1; // я вставил, потому что деление на ноль
				wx[0] = wx[0] * 200 / wy[0] + VIEWPORT_X / 2; wy[0] = wz[0] * 200 / wy[0] + VIEWPORT_Y / 2;
				wx[1] = wx[1] * 200 / wy[1] + VIEWPORT_X / 2; wy[1] = wz[1] * 200 / wy[1] + VIEWPORT_Y / 2;
				wx[2] = wx[2] * 200 / wy[2] + VIEWPORT_X / 2; wy[2] = wz[2] * 200 / wy[2] + VIEWPORT_Y / 2;
				wx[3] = wx[3] * 200 / wy[3] + VIEWPORT_X / 2; wy[3] = wz[3] * 200 / wy[3] + VIEWPORT_Y / 2;
				// draw points
				drawWall(wx[0], wx[1], wy[0], wy[1], wy[2], wy[3], W[w].c, s);
			}
			S[s].d /= (S[s].we - S[s].ws); // find average sector distance
			S[s].surface *= -1; // flip to negative to draw surface
		}		
	}	
}

int loadSectors[] =
{// wall start, wall end, z1 height, z2 height, bottom color, top color
	0, 4, 0, 40, 2, 3, // sector1
	4, 8, 0, 40, 4, 5, // sector2
	8, 12, 0, 40,6, 7, // sector3
	12,16,0, 40,0,1, // sector4
};

int loadWalls[] =
{ // x1,y1,x2,y2, color
	0,0,32,0,0,
	32,0,32,32,1,
	32,32,0,32,0,
	0,32,0,0,1,

	64,0,96,0,2,
	96,0,96,32,3,
	96,32,64,32,2,
	64,32,64,0,3,

	64,64,96,64,4,
	96,64,96,96,5,
	96,96,64,96,4,
	64,96,64,64,5,

	0,64,32,64,6,
	32,64,32,96,7,
	32,96,0,96,6,
	0,96,0,64,7
};

//-----------------------------------------------------------------------------
bool GameInit()
{
	for (int x = 0; x < 360; x++)
	{
		M.cos[x] = cos(x / 180.0 * M_PI);
		M.sin[x] = sin(x / 180.0 * M_PI);
	}

	P.x = 70;
	P.y = -110;
	P.z = 20;
	P.a = 0;
	P.l = 0;

	// load sectors
	int s, w, v1 = 0, v2 = 0;
	for (s = 0; s < numSect; s++)
	{
		S[s].ws = loadSectors[v1 + 0]; // wall start number
		S[s].we = loadSectors[v1 + 1]; // wall end number
		S[s].z1 = loadSectors[v1 + 2]; // sector bottom height
		S[s].z2 = loadSectors[v1 + 3] - loadSectors[v1+2]; // sector top height
		S[s].c1 = loadSectors[v1 + 4];
		S[s].c2 = loadSectors[v1 + 5];
		v1 += 6;

		for (w = S[s].ws; w < S[s].we; w++)
		{
			W[w].x1 = loadWalls[v2 + 0]; // bottom x1
			W[w].y1 = loadWalls[v2 + 1]; // bottom y1
			W[w].x2 = loadWalls[v2 + 2]; // bottom x2
			W[w].y2 = loadWalls[v2 + 3]; // bottom y2
			W[w].c = loadWalls[v2 + 4]; // wall color
			v2 += 5;

		}
	}

	return true;
}
//-----------------------------------------------------------------------------
void GameFrame()
{
	int x, y;
	if (T.fr1 - T.fr2 >= 50)                        //only draw 20 frames/second
	{
		clearBackground();
		movePlayer();
		draw3D();

		T.fr2 = T.fr1;
	}

	T.fr1 = SDL_GetTicks();          //1000 Milliseconds per second
}
//-----------------------------------------------------------------------------
void GameClose()
{
}
//-----------------------------------------------------------------------------