#include <iostream>
#include <vector>
#include <ctime>
#include <algorithm>
#include <thread>
#include <conio.h>
#include <cstdlib>
#include <chrono>

using namespace std;

// ========================================================= // 
// ======================= GAME TYPES ====================== // 
// ========================================================= // 

typedef vector <unsigned> line; // Grid line
typedef vector <line> matrix; // Grid

struct Position // Position in the grid
{
    unsigned x;
    unsigned y;
};

// ========================================================= // 
// ==================== GAME FUNCTIONS ===================== // 
// ========================================================= // 

void clearScreen() // Clear the terminal
{ 
    cout << "\033c";
}

unsigned colorPicker(const unsigned& num) // Color the grid's numbers in the terminal
{
    unsigned color;
    switch (num) 
    {
        case 0: color = 90; break; // grey
        case 1: color = 30; break; // black
        case 2: color = 31; break; // red
        case 3: color = 32; break; // green
        case 4: color = 33; break; // yellow
        case 5: color = 34; break; // blue
        case 6: color = 35; break; // magenta
        case 7: color = 36; break; // cyan
        case 8: color = 90; break; // grey
        case 9: color = 91; break; // orange
        default: color = 97; break; // white
    }
    return color;
}

matrix createMatrix(const size_t& size) // Create a grid of the given size filled with random numbers from 1 to 9
{
    matrix grid(size, line(size, 0));
    for (size_t i = 0; i < size; ++i)
    {
        for (size_t j = 0; j < size; ++j)
        {
            grid[i][j] = (rand() % 9) + 1;
        }
    }
    return grid;
}

void switchNumbers(matrix& grid, const Position& n1, const Position& n2) // Swap the numbers at the given positions in the grid
{
    swap(grid[n1.y][n1.x], grid[n2.y][n2.x]);
}

void displayMatrix(const matrix& grid, const Position& playerPos, bool isSelecting, const Position& selectedPos) // Display the grid in the terminal, highlighting the player's position and the selected cell if in selection mode
{
    for (size_t i = 0; i < grid.size(); ++i)
    {
        for (size_t j = 0; j < grid[i].size(); ++j)
        {
            bool isPlayer = (i == playerPos.y && j == playerPos.x);
            bool isSelected = (isSelecting && i == selectedPos.y && j == selectedPos.x);

            if (isPlayer)      cout << "[";
            else if (isSelected) cout << "<";
            else                 cout << " ";

            cout << "\033[" << colorPicker(grid[i][j]) << "m";
            
            if (grid[i][j] == 0) cout << ".";
            else                 cout << grid[i][j];
            
            cout << "\033[0m";

            if (isPlayer)      cout << "]";
            else if (isSelected) cout << ">";
            else                 cout << " ";
        }
        cout << endl;
    }
}

bool handleInput(Position& playerPos, const size_t& size) // Handle player input for movement and selection, returning true if the space key is pressed to confirm a selection
{
    int key = _getch(); 
    size_t gridHeight = size;
    size_t gridWidth = size;

    if (key == 0 || key == 224) 
    {
        key = _getch(); 
        switch (key) 
        {
            case 72: if (playerPos.y > 0) playerPos.y--; break;
            case 80: if (playerPos.y < gridHeight - 1) playerPos.y++; break;
            case 75: if (playerPos.x > 0) playerPos.x--; break;
            case 77: if (playerPos.x < gridWidth - 1) playerPos.x++; break;
        }
    }
    else 
    {
        switch (key) 
        {
            case 'z': case 'Z': if (playerPos.y > 0) playerPos.y--; break;
            case 's': case 'S': if (playerPos.y < gridHeight - 1) playerPos.y++; break;
            case 'q': case 'Q': if (playerPos.x > 0) playerPos.x--; break;
            case 'd': case 'D': if (playerPos.x < gridWidth - 1) playerPos.x++; break;
            case 32:  return true;
        }
    }
    return false;
}

bool matchLines(matrix& grid) // Check for matches of 3 or more identical numbers in a row or column, mark them for removal, and return true if any matches were found
{
    bool foundMatch = false;
    size_t size = grid.size();
    vector<vector<bool>> toRemove(size, vector<bool>(size, false));

    for (size_t i = 0; i < size; ++i)
    {
        for (size_t j = 0; j < size - 2; ++j) 
        {
            if (grid[i][j] != 0) 
            {
                size_t matchLength = 1;
                while (j + matchLength < size && grid[i][j] == grid[i][j + matchLength])
                {
                    matchLength++;
                }
                
                if (matchLength >= 3)
                {
                    foundMatch = true;
                    for (size_t k = 0; k < matchLength; ++k)
                    {
                        toRemove[i][j + k] = true;
                    }
                }
            }
        }
    }

    for (size_t j = 0; j < size; ++j)
    {
        for (size_t i = 0; i < size - 2; ++i)
        {
            if (grid[i][j] != 0)
            {
                size_t matchLength = 1;
                while (i + matchLength < size && grid[i][j] == grid[i + matchLength][j])
                {
                    matchLength++;
                }
                
                if (matchLength >= 3)
                {
                    foundMatch = true;
                    for (size_t k = 0; k < matchLength; ++k)
                    {
                        toRemove[i + k][j] = true;
                    }
                }
            }
        }
    }

    if (foundMatch)
    {
        for (size_t i = 0; i < size; ++i)
        {
            for (size_t j = 0; j < size; ++j)
            {
                if (toRemove[i][j])
                {
                    grid[i][j] = 0;
                }
            }
        }
    }

    return foundMatch;
}

void applyGravity(matrix& grid) // Apply gravity to the grid, making numbers fall down to fill empty spaces after matches have been removed
{
    size_t size = grid.size();

    for (size_t j = 0; j < size; ++j)
    {
        int fillIndex = size - 1; 

        for (int i = size - 1; i >= 0; --i)
        {
            if (grid[i][j] != 0)
            {
                grid[fillIndex][j] = grid[i][j];
                fillIndex--;
            }
        }

        while (fillIndex >= 0)
        {
            grid[fillIndex][j] = 0;
            fillIndex--;
        }
    }
}

bool isPlayable(const matrix& grid) // Check if there are any possible moves left by counting the occurrences of each number and returning true if any number appears at least 3 times
{
    vector<int> counts(10, 0); 

    for (size_t i = 0; i < grid.size(); ++i)
    {
        for (size_t j = 0; j < grid[i].size(); ++j)
        {
            unsigned val = grid[i][j];
            if (val != 0)
            {
                counts[val]++;
                if (counts[val] >= 3)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

// ========================================================= // 
// ========================= GAME ========================== // 
// ========================================================= //
int main() 
{
    srand(time(0)); 

// ==================== VARIABLES ===================== //
    int movesLeft = 15; 
    int score = 0;
    Position playerPos{0, 0}; 
    matrix grid = createMatrix(5); 

    bool isSelecting = false; 
    Position selectedPos{0, 0}; 

// ==================== GAME LOOP ===================== //
    while (matchLines(grid)) 
    {
        applyGravity(grid);
    }

    while (movesLeft > 0 && isPlayable(grid)) 
    {
        clearScreen(); 

        cout << "Moves left: " << movesLeft << "   |   Score: " << score << "\n\n";
        
        if (isSelecting) {
            cout << "\033[33m[SELECTION MODE] Move to the target and confirm (SPACE)\033[0m\n\n";
        } else {
            cout << "Move using arrow keys or ZQSD. Press SPACE to select a cell to swap.\n\n";
        }

        displayMatrix(grid, playerPos, isSelecting, selectedPos); 

        bool spacePressed = handleInput(playerPos, 5);

        if (spacePressed)
        {
            if (!isSelecting) 
            {
                selectedPos = playerPos;
                isSelecting = true;
            } 
            else 
            {
                switchNumbers(grid, selectedPos, playerPos);
                isSelecting = false;
                movesLeft--; 

                bool combo = true;
                while (combo)
                {
                    if (matchLines(grid))
                    {
                        score += 10; 

                        clearScreen();
                        cout << "Moves left: " << movesLeft << "   |   Score: " << score << "\n\n";
                        cout << "\033[32mCOMBO!\033[0m\n\n";
                        displayMatrix(grid, playerPos, false, selectedPos);
                        this_thread::sleep_for(chrono::milliseconds(400));

                        applyGravity(grid); 
                    }
                    else
                    {
                        combo = false; 
                    }
                }
            }
        }
    }
    
    // ==================== ENDGAME ===================== //
    clearScreen();
    displayMatrix(grid, playerPos, false, selectedPos); 
    cout << "\n--- GAME OVER ---\n";
    
    if (movesLeft <= 0) {
        cout << "You are out of moves!\n";
    } else {
        cout << "There are not enough identical numbers left to make a match!\n";
    }
    
    cout << "Your final score is: " << score << " points.\n";
    
    return 0;
}
