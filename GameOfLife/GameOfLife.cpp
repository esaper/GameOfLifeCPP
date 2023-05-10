#include <iostream>
#include <map>
#include <set>
#include <string>
#include <time.h>
#define SDL_MAIN_HANDLED
#include "SDL.h"

using namespace std;

struct cellLoc
{
	int x, y;

	// I don't pretend to quite understand the following, but it is necessary for this struct to work as the key to a map.
	// Adapted from https://dawnarc.com/2019/09/c-how-to-use-a-struct-as-key-in-a-std-map/
	bool operator == (const cellLoc& o) const
	{
		return x == o.x && y == o.y;
	}

	bool operator < (const cellLoc& o) const
	{
		return x < o.x || (x == o.x && y < o.y);
	}
};

struct cellData
{
	int currState, nextState, numNeighbors;
};

struct ruleSet
{
	set<int> birthList, surviveList;
};

struct color
{
	unsigned int r, g, b;
};

const color colors[] = { {0, 0, 0}, {255, 255, 255} };

// Constants
const unsigned int WIDTH = 1900;
const unsigned int HEIGHT = 1000;
unsigned int CELL_SIZE = 5;
unsigned int FRAME_DELAY = 0;

// Initial display range
int numRows = HEIGHT / (CELL_SIZE + 1);
int numCols = WIDTH / (CELL_SIZE + 1);
cellLoc topLeft = { -(numCols / 2), -(numRows / 2) };
cellLoc	center = { 0, 0 };

map<string, ruleSet> RULES =
{
	{"Conway's Game of Life", {{3}, {2, 3}}},
	{"3-4 Life", {{3, 4}, {3, 4}}},
	{"Amoeba", {{3, 5, 7}, {1, 3, 5, 8}}},
	{"Coagulations" , {{3, 7, 8}, {2, 3, 5, 6, 7, 8}}},
	{"Coral" , {{3}, {4, 5, 6, 7, 8}}},
	{"Corrosion of Conformity" , {{3}, {1, 2, 4}}},
	{"Day & Night" , {{3, 6, 7, 8}, {3, 4, 6, 7, 8}}},
	{"Life Without Death" , {{3}, {0, 1, 2, 3, 4, 5, 6, 7, 8}}},
	{"Gnarl" , {{1}, {1}}},
	{"High Life" , {{3, 6}, {2, 3}}},
	{"Inverse Life" , {{0, 1, 2, 3, 4, 7, 8}, {3, 4, 6, 7, 8}}},
	{"Long Life" , {{3, 4, 5}, {5}}},
	{"Maze" , {{3}, {1, 2, 3, 4, 5}}},
	{"Mazectric" , {{3}, {1, 2, 3, 4}}},
	{"Pseudo Life" , {{3, 5, 7}, {2, 3, 8}}},
	{"Replicator" , {{1, 3, 5, 7}, {1, 3, 5, 7}}},
	{"Seeds" , {{2}, {}}},
	{"Serviettes" , {{2, 3, 4}, {}}},
	{"Stains" , {{3, 6, 7, 8}, {2, 3, 5, 6, 7, 8}}},
	{"Walled Cities" , {{3, 6, 7, 8}, {2, 3, 5, 6, 7, 8}}}
};

// Select ruleset to use
const string ruleName = "Conway's Game of Life";
const ruleSet rules = RULES[ruleName];

map<cellLoc, cellData> cells;
set<cellLoc> cellsToUpdate, cellsToRemove;

int frame = 0;
int liveCells = 0;

// Function declarations
void createRandom();
void drawCell(int x, int y, int state);
void moveScreen(cellLoc centerPoint);
void removeCells();
void setNextState();
void toggleCell(cellLoc mousePos);
void updateCell(cellLoc cell);

// Graphics
SDL_Window* window = NULL;
SDL_Surface* surface = NULL;


long timediff(clock_t t1, clock_t t2) {
	long elapsed;
	elapsed = ((long)t2 - t1) / CLOCKS_PER_SEC * 1000;
	return elapsed;
}


void createRandom()
{
	int gridX, gridY;
	// Create random cells within visible window
	for (int i = 0; i < numCols * numRows; i++)
	{
		gridX = (rand() % numCols) + topLeft.x;
		gridY = (rand() % numRows) + topLeft.y;
		if (cells.find({ gridX, gridY }) == cells.end())
		{
			cells[{gridX, gridY}] = { 1, 1, 0 };
		}
		else
		{
			cells[{gridX, gridY}].nextState = 1 - cells[{gridX, gridY}].currState;
		}
		updateCell({ gridX, gridY });
	}
	SDL_UpdateWindowSurface(window);
}

void drawCell(int x, int y, int state)
{
	// No need to draw if cell does not appear in the window
	if (x >= topLeft.x && x < (topLeft.x + numCols) && y >= topLeft.y && y < (topLeft.y + numRows))
	{
		// Calculate screen offsets
		int screen_x = (x - topLeft.x); // *(CELL_SIZE + 1);
		int screen_y = (y - topLeft.y); // *(CELL_SIZE + 1);

		// Draw the cell
		Uint8* pixel_ptr = (Uint8*)surface->pixels + (screen_y * (CELL_SIZE + 1) * WIDTH + screen_x * (CELL_SIZE + 1)) * 4;

		for (unsigned int i = 0; i < CELL_SIZE; i++)
		{
			for (unsigned int j = 0; j < CELL_SIZE; j++)
			{
				*(pixel_ptr + j * 4) = colors[state].r;
				*(pixel_ptr + j * 4 + 1) = colors[state].g;
				*(pixel_ptr + j * 4 + 2) = colors[state].b;
			}
			pixel_ptr += WIDTH * 4;
		}
	}
}

void moveScreen(cellLoc centerPoint)
{
	numRows = HEIGHT / (CELL_SIZE + 1);
	numCols = WIDTH / (CELL_SIZE + 1);
	topLeft = { (centerPoint.x - (numCols / 2)), (centerPoint.y - (numRows / 2)) };
	// Clear the screen
	SDL_memset(surface->pixels, 0, surface->h * surface->pitch);

	// Redraw active cells
	for (map<cellLoc, cellData>::iterator it = cells.begin(); it != cells.end(); it++)
	{
		if (it->second.currState == 1)
		{
			drawCell(it->first.x, it->first.y, 1);
		}
	}
	SDL_UpdateWindowSurface(window);
}

void removeCells()
{
	// Remove inactive cells with no neighbors
	for (set<cellLoc>::iterator loc = cellsToRemove.begin(); loc != cellsToRemove.end(); loc++)
	{
		if (cells.find(*loc) != cells.end() && cells[*loc].currState == 0 && cells[*loc].numNeighbors == 0)
		{
			cells.erase(*loc);
		}
	}
}

void setNextState()
{
	for (map<cellLoc, cellData>::iterator it = cells.begin(); it != cells.end(); it++)
	{
		if (it->second.currState == 1)
		{
			if (rules.surviveList.find(it->second.numNeighbors) == rules.surviveList.end())
			{
				it->second.nextState = 0;
				cellsToUpdate.insert(it->first);
			}
		}
		else if (rules.birthList.find(it->second.numNeighbors) != rules.birthList.end())
		{
			it->second.nextState = 1;
			cellsToUpdate.insert(it->first);
		}
	}
}

void toggleCell(cellLoc mousePos)
{
	int x = mousePos.x / (CELL_SIZE + 1) + topLeft.x;
	int y = mousePos.y / (CELL_SIZE + 1) + topLeft.y;
	if (cells.find({ x, y }) == cells.end())
	{
		// Create new cell
		cells[{x, y}] = { 1, 1, 0 };
	}
	else
	{
		cells[{x, y}].nextState = 1 - cells[{x, y}].currState;
	}
	updateCell({ x, y });
	removeCells();
	SDL_UpdateWindowSurface(window);
}

void updateCell(cellLoc cell)
{
	int x0 = cell.x, y0 = cell.y;
	cells[cell].currState = cells[cell].nextState;
	// Update each of current cell's neighbors' num_neighbors based on current cell's state
	if (cells[cell].currState == 1)
	{
		for (int y = y0 - 1; y < y0 + 2; y++)
		{
			for (int x = x0 - 1; x < x0 + 2; x++)
			{
				if (cells.find({ x, y }) == cells.end())
				{
					// if cell doesn't exist, create cell for neighbor
					cells[{x, y}] = { 0, 0, 1 };
				}
				else
				{
					// Add 1 to neighbor count
					cells[{x, y}].numNeighbors++;
				}
			}
		}
		cells[{x0, y0}].numNeighbors--;  // Don't count self
		liveCells++;
	}
	else
	{
		for (int y = y0 - 1; y < y0 + 2; y++)
		{
			for (int x = x0 - 1; x < x0 + 2; x++)
			{
				if (x != x0 || y != y0)
				{
					cells[{x, y}].numNeighbors--;
				}
				if (cells[{x, y}].currState == 0 && cells[{x, y}].numNeighbors == 0)
				{
					// Neighbor is inactive and has no active neighbors, so remove
					cellsToRemove.insert({ x, y });
				}
			}
		}
		liveCells--;
	}
	drawCell(x0, y0, cells[cell].currState);
}

int main()
{
	// Initialize window
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("Conway's Game of Life", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
	surface = SDL_GetWindowSurface(window);

	string title;
	srand((unsigned int)time(0));	// Random seed
	bool running = true;
	bool paused = true;
	bool singleFrame = false;
	clock_t t1, t2;
	long elapsed;

	t1 = clock();

	// Event Handler
	SDL_Event event;

	while (running)
	{
		// Check events
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				running = false;
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
				case SDLK_ESCAPE:
					running = false;
					break;
				case SDLK_SPACE:
					paused = !paused;
					break;
					// Arrow keys move display over 10%
				case SDLK_LEFT:
					center = { center.x - (numCols / 10), center.y };
					moveScreen(center);
					break;
				case SDLK_RIGHT:
					center = { center.x + (numCols / 10), center.y };
					moveScreen(center);
					break;
				case SDLK_UP:
					center = { center.x, center.y - (numCols / 10) };
					moveScreen(center);
					break;
				case SDLK_DOWN:
					center = { center.x, center.y + (numCols / 10) };
					moveScreen(center);
					break;
					// PgUp/PgDn change block size
				case SDLK_PAGEUP:
					CELL_SIZE++;
					moveScreen(center);
					break;
				case SDLK_PAGEDOWN:
					if (CELL_SIZE > 1)
					{
						CELL_SIZE--;
						moveScreen(center);
					}
					break;
					// +/- Change frame rate
				case SDLK_KP_PLUS:
					FRAME_DELAY /= 1.2;
					break;
				case SDLK_KP_MINUS:
					FRAME_DELAY *= 1.2;
					break;
				case SDLK_r:
					createRandom();
					break;
					// Advance a single frame
				case SDLK_s:
					paused = false;
					singleFrame = true;
					break;
				}
			case SDL_MOUSEBUTTONDOWN:
				toggleCell({ event.button.x, event.button.y });
			}
		}


		if (!paused)
		{
			frame++;

			// Clear prior to each generation
			cellsToUpdate.clear();
			cellsToRemove.clear();

			// Evaluate all cells for next state
			setNextState();

			// Update cells
			if (cellsToUpdate.size() == 0)  // Nothing changed--stable state
			{
				paused = true;
			}
			else
			{
				for (set<cellLoc>::iterator loc = cellsToUpdate.begin(); loc != cellsToUpdate.end(); loc++)
				{
					if (cells.find(*loc) != cells.end())
					{
						updateCell(*loc);
					}
				}
			}

			//Remove inactive cells with no neighbors
			removeCells();

			if (liveCells == 0)
			{
				paused = true;
			}

			if (singleFrame)
			{
				paused = true;
				singleFrame = false;
			}

			SDL_UpdateWindowSurface(window);
			SDL_Delay(FRAME_DELAY);

			t2 = clock();
			elapsed = timediff(t1, t2);
			title = ruleName + "    Current Frame: " + to_string(frame) + "     FPS: " + to_string(1000.0 / elapsed) + "     Live Cells: " + to_string(liveCells) + "     Eval List: " + to_string(cells.size()) +
				"     Updates: " + to_string(cellsToUpdate.size()) + "     Center: (" + to_string(center.x) + ", " + to_string(center.y) + ")";
			SDL_SetWindowTitle(window, title.c_str());
			t1 = t2;

			/*
			if (frame % 100 == 0)
			{
				cout << "Frame: " << frame << "   Cells: " << liveCells << endl;
			}
			*/
		}
	}

	// Shut down SDL
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
