#include <iostream>
#include <list>
#include <map>
#include <set>
#include <string>
#include <time.h>
#include <tuple>


using namespace std;


typedef tuple<int, int> cellLoc;
typedef tuple<int, int, int> cellData;


struct ruleSet
{
	set<int> birthList, surviveList;
};


// Constants
const int WIDTH = 1900;
const int HEIGHT = 1000;
const int CELL_SIZE = 7;
const float FPS = 1000.0;

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
	// Create random cells within visible window
	for (int i = 0; i < numCols * numRows; i++)
	{
		int gridX = (rand() % numCols) + get<0>(topLeft);
		int gridY = (rand() % numRows) + get<1>(topLeft);

	}
}



int main()
{
	/*
	Initialize window
	*/

	srand(time(0));
	bool running = true;
	bool paused = true;
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
	}
}
