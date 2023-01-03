#include <iostream>
#include <map>
#include <set>
#include <string>
#include <time.h>
#include <tuple>


using namespace std;


//typedef tuple<int, int> cellLoc;
//typedef tuple<int, int, int> cellData;

struct cellLoc
{
	int x, y;

	// I don't pretend to quite understand the following, but it is necessary for this struct to work as the key to a map.
	// Adapted from https://dawnarc.com/2019/09/c-how-to-use-a-struct-as-key-in-a-std-map/
	bool operator==(const cellLoc& o) const
	{
		return x == o.x && y == o.y;
	}

	bool operator<(const cellLoc& o)const
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


// Constants
const int WIDTH = 1900;
const int HEIGHT = 1000;
const int CELL_SIZE = 7;
const float FPS = 1000.0;

const int CURR_STATE = 0;
const int NEXT_STATE = 1;
const int NUM_NEIGHBORS = 2;


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

string ruleName = "Conway's Game of Life";
ruleSet rules = RULES[ruleName];


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


void createRandom() 
{
	int gridX, gridY;
	// Create random cells within visible window
	for (int i = 0; i < numCols * numRows; i++)
	{
		gridX = (rand() % numCols) + topLeft.x;
		gridY = (rand() % numRows) + topLeft.y;
		if (cells.find({gridX, gridY}) == cells.end())
		{
			cells[{gridX, gridY}] = { 1, 1, 0 };
		}
		else
		{
			cells[{gridX, gridY}].nextState = 1 - cells[{gridX, gridY}].currState;
		}
		updateCell({gridX, gridY});
	}
	/*
	Flip screen
	*/
}


void drawCell(int x, int y, int state)
{
	// No need to draw if cell does not appear in the window
	if (x >= topLeft.x && x < (topLeft.x + numCols) && y >= topLeft.y && y < (topLeft.y + numRows))
	{
		// Calculate screen offsets
		int screen_x = (x - topLeft.x) * (CELL_SIZE + 1);
		int screen_y = (y - topLeft.y) * (CELL_SIZE + 1);
		/*
		Draw the cell
		*/
	}
}



void moveScreen(cellLoc centerPoint)
{
	numRows = HEIGHT / (CELL_SIZE + 1);
	numCols = WIDTH / (CELL_SIZE + 1);
	topLeft = { (centerPoint.x - (numCols / 2)), (centerPoint.y - (numRows / 2)) };
	// Clear the screen
	/*
	Clear the screen
	*/
	// Redraw active cells
	for (map<cellLoc, cellData>::iterator it = cells.begin(); it != cells.end(); it++)
	{
		if (it->second.currState == 1)
		{
			drawCell(it->first.x, it->first.y, 1);
		}
	}
	/*
	Flip screen
	*/
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
	int x = mousePos.x;	// / (CELL_SIZE + 1) + topLeft.x;
	int y = mousePos.y;	// / (CELL_SIZE + 1) + topLeft.y;
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
	/*
	Flip screen
	*/
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
	/*
	Initialize window
	*/

	/*
	Glider
	*/
	toggleCell({ 0, 0 });
	toggleCell({ 1, 0 });
	toggleCell({ 2, 0 });
	toggleCell({ 2, 1 });
	toggleCell({ 1, 2 });
	/*
	*/

	srand((unsigned int)time(0));	// Random seed
	bool running = true;
	bool paused = false;	// Should normally be initialized to true
	bool singleFrame = false;

	while (running)
	{
		/*
		Check events
		*/

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

			/*
			Update display
			Update window caption
			*/
		}
		if (frame % 1000 == 0)
		{
			cout << "Frame: " << frame << "   Cells: " << liveCells << endl;
		}
	}
}
