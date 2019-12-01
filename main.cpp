//------------------------------------
//  vexed.cpp
//     Move pieces to be adjacent.  Once adjacent pieces match, they are cleared off the board.  Once the entire
//     board is clear, play advances to the next level.
//
//  Prog 6: Vexed with Undo
//  Author: Tanmay Sarin
//  Using Codio, for UIC CS 141, Fall 2019
//
//  Running the program looks like:
/*
 

 */
#include <iostream>   // For cin, cout
#include <fstream>    // For file input
#include <cstdlib>    // For system("clear") used to clear the screen
#include <chrono>     // Used in pausing for some milliseconds using sleep_for(...)
#include <thread>     // Used in pausing for some milliseconds using sleep_for(...)
using namespace std;

// Global constants
const int MaxNumberOfBoards = 118;
const int BoardRows = 8;
const int BoardColumns = 10;
const int NumberOfPresetBoardParValues = 60;    // After the first 60 par values, the default par value is 15
const char FileName[] = "boards.txt";
const int SleepAmount = 85;                    // Length of time to pause, in milliseconds
const int ExtraSleepAmountAfterDelete = 350;   // Extra pause after deleting a matching set of pieces
const int ExtraSleepAmountAfterCompletingLevel = 1900;  // Extra pause after completing a level

// Global variable
bool UseSystemClear = false; // Initial prompt can reset this to true.


//------------------------------------------------------------------------------------------------------------------
// Class used to store a game board
class Board
{
    public:
        // Constructor declarations.  See below for the definitions.
        Board();  // Default constructor
        Board( int theBoardNumber, int theBoard[ BoardRows * BoardColumns], int theParValue); // Fully qualified constructor
        Board( const Board &existingBoard);    // Copy constructor
    
        // Get and Set member functions
        int  getBoardNumber()             { return boardNumber;        }
        char getPiece( int row, int col)  { return board[ row][ col];  }
        int  getParValue()                { return parValue;           }
        int  getMoveNumber()              { return moveNumber;         }
        void setPiece( int row, int col,
                       char character)    { board[ row][ col] = character; }
    
        // Utility member function declarations.  See below for their definitions.
        char translate( int n);             // Translate the stored board integer values into characters
        int  reverseTranslate( char c);     // Translate board display values into corresponding integer values
        void displayBoard();                // Display the board
        void getUserInputAndMakeMove( char &moveAction);    // Get user input and make move. Return move Action for 'X','R', and 'S'
        void slidePieceDown( int row, int col);             // Slide the piece at row,column down as far as it will go
        void slideAllPiecesDown();          // Slide all pieces that have space below them down as far as they can go.
        void markMatchingPieces( int pieceToMatch, int row, int col, bool &matchesFound);    // Mark matching pieces with -1
        void blankOutMatchingPieces();      // Blank out matching pieces previously marked with -1
        void findAndDeleteMatchingPieces(); // Find adjacent matching pieces throughout the board.
        bool boardIsClear();                // Returns true if board is cleared of all pieces; false otherwise
        bool boardClearedWithinParMoves();  // Returns true if board is cleared within par moves; false otherwise
        bool isNotTheSameAs( Board otherBoard); // Returns true if caller board is not the same as parameter board; false otherwise

    private:
        int boardNumber;
        int moveNumber;
        int board[ BoardRows][ BoardColumns];   // 2D array is used since we only ever move one square left or right
        int parValue;
};  // end class Board


//------------------------------------------------------------------------------------------------------------------
// Board class default constructor
Board::Board()
{
    // Do an element-by-element copy of the board values
    for( int row=0; row<BoardRows; row++) {
        for( int col=0; col<BoardColumns; col++) {
            board[ row][ col] = -1;
        }
    }
    
    // Also set the board number, par value, and move number for this level
    boardNumber = -1;
    parValue = -1;
    moveNumber = 0;
} // end Board::Board()


//------------------------------------------------------------------------------------------------------------------
// Board class fully qualified constructor
Board::Board( int theBoardNumber, int theBoard[ BoardRows * BoardColumns], int theParValue)
{
    // Set the board number
    boardNumber = theBoardNumber;
    
    // Do an element-by-element copy of the board values
    for( int row=0; row<BoardRows; row++) {
        for( int col=0; col<BoardColumns; col++) {
            board[ row][ col] = theBoard[ row * BoardColumns + col];
        }
    }
    
    // Also set the par value and starting move number for this level
    parValue = theParValue;
    moveNumber = 0;
} // end Board::Board(...)


//------------------------------------------------------------------------------------------------------------------
// Board class copy constructor
Board::Board( const Board &existingBoard)
{
    // Set the board number, par value and move number
    boardNumber = existingBoard.boardNumber;
    parValue = existingBoard.parValue;
    moveNumber = existingBoard.moveNumber;
    
    // Do an element-by-element copy of the board values
    for( int row=0; row<BoardRows; row++) {
        for( int col=0; col<BoardColumns; col++) {
            board[ row][ col] = existingBoard.board[ row][ col];
        }
    }

} // end Board::Board(...)

//------------------------------------------------------------------------------------------------------------------
// Board class utility function.
// Translate the board number to a friendlier display character.
char Board::translate( int n) {
    char c;
    switch( n) {
        case 0: c = ' '; break;
        case 1: c = '&'; break;
        case 2: c = '@'; break;
        case 3: c = '+'; break;
        case 4: c = '%'; break;
        case 5: c = '^'; break;
        case 6: c = '#'; break;
        case 7: c = '='; break;
        case 8: c = '*'; break;
        case 9: c = '.'; break;
        default: cout << "Invalid value " << n << " sent to translate().  Exiting...";
                 exit( -1);
                 break;
     } //end switch( n)
     return c;
} // end Board::translate()


//------------------------------------------------------------------------------------------------------------------
// Board class utility function.
// Translate the board character to the underlying numerical value.
int Board::reverseTranslate( char c) {
    int n;
    switch( c) {
        case ' ': n = 0; break;
        case '&': n = 1; break;
        case '@': n = 2; break;
        case '+': n = 3; break;
        case '%': n = 4; break;
        case '^': n = 5; break;
        case '#': n = 6; break;
        case '=': n = 7; break;
        case '*': n = 8; break;
        case '.': n = 9; break;
        default: cout << "Invalid value " << c << " sent to reverseTranslate().  Exiting...";
                 exit( -1);
                 break;
     } //end switch( c)
     return n;
} // end Board::reverseTranslate()


//------------------------------------------------------------------------------------------------------------------
// Board class utility function.
// Display the board
void Board::displayBoard() {
    // Clear the screen and pause the program for designated amount of time
    std::this_thread::sleep_for(std::chrono::milliseconds( SleepAmount));
    // Check global variable to determine whether or not to clear the screen
    if( UseSystemClear) {
        system( "clear");
    }

    cout << endl
         << "    Board " << boardNumber << " par " << parValue << endl;
    
    // Display the board
    cout << "      0 1 2 3 4 5 6 7 8 9  " << endl
         << "    -----------------------" << endl;
    for( int row=0; row<BoardRows; row++) {
        char rowLetter = 'A'+row;
        cout << "  " << rowLetter << " | ";
        for( int col=0; col<BoardColumns; col++) {
            cout << translate( board[ row][ col]) << " ";
        }
        cout << "| " << rowLetter << endl;
    }
    cout << "    -----------------------" << endl
         << "      0 1 2 3 4 5 6 7 8 9  " << endl;
} // end Board:: displayBoard()


//------------------------------------------------------------------------------------------------------------------
// Board class utility function.
// Get user input and make move.  This also handles special user input to handle cases of 'U' to unde the most
// recent move, 'X' to exit the program, 'R' to reset the current level, and 'S' to set the board to a new level.
void Board::getUserInputAndMakeMove( char &moveAction)
{
    char rowLetter, columnCharacter, direction;
    int row, col;   // Will store the numerical index equivalent of the user input letters (e.g. 'A' = 0, 'B' = 1, etc)
    
    // Use a loop to allow using continue after invalid user input to reprompt for the move
    while( true) {
        // Display the moveNumber+1 in the prompt, but don't actually increment the value until after we know we
        // are making a move, and not handling some case such as reset, set, or exit.
        cout << moveNumber+1 << ". Your move: ";
        cin >> rowLetter;
        rowLetter = toupper( rowLetter);
        
        // See if 'X' was entered, to exit the program
        if( rowLetter == 'X') {
            exit( 0);
        }
        
        // See if 'R' was entered to retry this level
        if( rowLetter == 'R') {
            moveAction = 'R';
            return;
        }
        
        // See if 'S' was entered to set the level
        if( rowLetter == 'S') {
            moveAction = 'S';
            return;
        }
        
        // See if 'U' was entered to undo the most recent move
        if( rowLetter == 'U') {
            moveAction = 'U';
            return;
        }
        
        // Now also read in the column and direction
        cin >> columnCharacter >> direction;
        // Convert these to upper case as well
        columnCharacter = toupper( columnCharacter);
        direction = toupper( direction);
        
        // Convert the user input letters to numerical values used to index the board array
        row = rowLetter - 'A';
        col = columnCharacter - '0';
        
        // Validate row, col values are in bounds
        if( row < 0 || row > (BoardRows-1) || col < 0 || col > (BoardColumns-1) ) {
            cout << "Move value is out of bounds.  Please retry." << endl;
            continue;
        }
        
        // Ensure character to move is not a wall or a space
        if( translate( board[ row][ col]) == '.' || translate( board[ row][ col]) == ' ') {
            cout << "Attempting to move an invalid character.  Please retry. " << endl;
            continue;   // Loop back up to reprompt for the move
        }
        
        // Validate the destination square is empty
        int newCol = col;
        if( direction == 'L') {
            newCol--;
        }
        else if( direction == 'R') {
            newCol++;
        }
        else {
            cout << "Invalid move direction.  Please retry." << endl;
            continue;
        }
        if( translate( board[ row][ newCol]) != ' ') {
            cout << "Attempting to move into a non-empty space.  Please retry." << endl;
            continue;
        }
        
        // Now that we have user input, make the move and increment the move number
        board[ row][ newCol] = board[ row][ col];
        board[ row][ col] = reverseTranslate( ' ');     // Store the number corresponding to ' '
        this_thread::sleep_for(chrono::milliseconds( SleepAmount));    // Sleep
        displayBoard();
        
        moveNumber++;   // Increment the move number, now that we know we are making a move.
        
        break;  // break out of the enclosing move-making loop
    } // end while( true)

} // end Board::getUserInputAndMakeMove()


//------------------------------------------------------------------------------------------------------------------
// Board class utility function.
// Slide the piece at row,column down as far as it will go
void Board::slidePieceDown( int row, int col)
{
    // While the piece below is empty, slide it down.
    while( translate( board[ row][ col]) != '.' &&   // Current piece is not a wall
           translate( board[ row][ col]) != ' ' &&   // Current piece is not a space
           translate( board[ row+1][ col]) == ' '    // Square below current piece is empty
         ) {
        // Slide it down
        board[ row+1][ col] = board[ row][ col];
        board[ row][ col] = reverseTranslate(' ');
        // See if it can be slid down even further
        row++;
        
        this_thread::sleep_for(chrono::milliseconds( SleepAmount));    // Sleep
        displayBoard();
    }
} // end Board::slidePieceDown( ...)


//------------------------------------------------------------------------------------------------------------------
// Board class utility function.
// Slide all pieces that have space below them down as far as they can go.
// Start at the bottom of the array and work backwords, so that pieces below slide first
// and don't get in the way of pieces above.
void Board::slideAllPiecesDown()
{
    // Start at the last board location, at row 6 column 8
    for( int row = BoardRows - 2; row > 0; row--) {
        for( int col = BoardColumns - 2;  col > 0; col--) {
            slidePieceDown( row, col);
        }
    }
} // end Board::slideAllPiecesDown()


//------------------------------------------------------------------------------------------------------------------
// Board class utility function.
// Erase adjacent matching pieces for a particular board square, then recursively continue to
// do so for any adjacent piece that matches.  Keep track of matching squares by setting them to -1.
// Finally go through and set all -1 values to be the numerical value that corresponds to ' ' (which is 0)
// Set matchesFound to true if any matches are found.
void Board::markMatchingPieces( int pieceToMatch, int row, int col, bool &matchesFound)
{
    // Check each neighbor, and set it to -1 if it matches. If it matches and is set to -1,
    // then recursively repeat the process, from the perspective of that neighbor.
    // First check the left neighbor:
    if( board[ row][ col-1] == pieceToMatch && translate(board[ row][ col-1]) != '.') {
        board[ row][ col-1] = -1;
        matchesFound = true;
        markMatchingPieces( pieceToMatch, row, col-1, matchesFound);
    }
    // Now check the right neighbor:
    if( board[ row][ col+1] == pieceToMatch && translate(board[ row][ col+1]) != '.') {
        board[ row][ col+1] = -1;
        matchesFound = true;
        markMatchingPieces( pieceToMatch, row, col+1, matchesFound);
    }
    // Now check the above neighbor:
    if( board[ row-1][ col] == pieceToMatch && translate(board[ row-1][ col]) != '.') {
        board[ row-1][ col] = -1;
        matchesFound = true;
        markMatchingPieces( pieceToMatch, row-1, col, matchesFound);
    }
     // Now check the below neighbor:
    if( board[ row+1][ col] == pieceToMatch && translate(board[ row+1][ col]) != '.') {
        board[ row+1][ col] = -1;
        matchesFound = true;
        markMatchingPieces( pieceToMatch, row+1, col, matchesFound);
    }
}// end Board::markMatchingPieces()


//------------------------------------------------------------------------------------------------------------------
// Board class utility function.
// Board class: Blank out matching pieces previously marked with -1
void Board::blankOutMatchingPieces()
{
    // Go through each playable position on the board
    for( int row = 1; row < BoardRows - 1; row++) {
        for( int col = 1; col < BoardColumns - 1; col++) {

            if( board[ row][ col] == -1) {
                // Board position was previously marked as matching, so should be blanked out.
                board[ row][ col] = reverseTranslate( ' ');      // Set board value to number that represents space.
            }
        }
    } //end for( int row...
} // end Board::blankOutMatchingPieces()


//------------------------------------------------------------------------------------------------------------------
// Board class utility function.
// Find and delete adjacent matching pieces throughout the entire board.
void Board::findAndDeleteMatchingPieces()
{
    // Look at each board piece, comparing it to its adjacent neighbors.
    // Only look at rows 1..6 (not row 0 or row 7)
    for( int row = 1; row < BoardRows - 1; row++) {
        for( int col = 1; col < BoardColumns - 1; col++) {
        
            bool matchesFound = false;
            // Only check for matching pieces for board elements that are not spaces or walls.
            char c = translate( board[ row][ col]);
            if( c != '.' && c != ' ') {
                // Recursively mark any neighbor that matches as -1.  Set matchesFound to true if any are found.
                markMatchingPieces( board[ row][ col], row, col, matchesFound);
            }
            
            // Go through board and blank out pieces marked (with -1) as matching.
            blankOutMatchingPieces();
            
            // Display the board after matching pieces were found and blanked out
            if( matchesFound) {
                // Pause the program for designated amount of time, in addition to the built-in pause in displayBoard()
                std::this_thread::sleep_for(std::chrono::milliseconds( ExtraSleepAmountAfterDelete));
                
                displayBoard();
            }
        }
    } //end for( int row...
} // end Board::lookForMatchingPieces()


//------------------------------------------------------------------------------------------------------------------
// Board class utility function.
// Returns true when board is cleared of all pieces; false otherwise
bool Board::boardIsClear()
{
    // Look at each board piece, comparing it to its adjacent neighbors.
    // Only look at rows 1..6 (not row 0 or row 7)
    for( int row = 1; row < BoardRows - 1; row++) {
        for( int col = 1; col < BoardColumns - 1; col++) {
            // Get the character at this row,col position
            char c = translate( board[ row][ col] );
            if( c != ' ' && c != '.') {
                // There is still some piece on the board, so board is not clear
                return false;
            }
        }
    } //end for( int row...
    
    return true;
} // end Board::boardIsClear()


//------------------------------------------------------------------------------------------------------------------
// Board class utility function.
// Returns true when board was cleared within par moves; false otherwise
bool Board::boardClearedWithinParMoves()
{
    if( boardIsClear() && moveNumber <= getParValue() ) {
        return true;
    }
    
    return false;
} // end Board::boardClearedWithinParMoves()


//------------------------------------------------------------------------------------------------------------------
// Board class utility function.
// Returns true when board pieces of calling object are the same as the board pieces of the parameter board
bool Board::isNotTheSameAs( Board otherBoard)
{
    // Compare each board member
    for( int row = 0; row < BoardRows; row++) {
        for( int col = 0; col < BoardColumns; col++) {
            if( board[ row][ col] != otherBoard.board[ row][ col]) {
                // This board position piece is different, so they are not the same
                return true;
            }
        }
    } //end for( int row...

    return false;    // All pieces were the same
}


//------------------------------------------------------------------------------------------------------------------
// Class used to read in and store all game boards
class AllBoards
{
    public:
        //------------------------------------------------------------------------------------------------------------------
        // Constructor that reads in data from the data file.
        AllBoards();    // Declaration.  See the definition outside the class, below.
    
        // Get and Set member functions.
    
        //------------------------------------------------------------------------------------------------------------------
        // Find and return a particular board. Boards are stored using 0-based indexing, but for the user interface
        // we use 1-based indexing, so we need to subtract 1 from it to get the stored board corresponding to
        // the desired boardIndex.
        Board getBoard( int boardIndex)
        {
            // Subtract 1 from boardIndex to compensate for 0 vs 1 based indexing (see comment above)
            boardIndex--;
            
            // Validate board index number
            if( boardIndex < 0 || boardIndex > MaxNumberOfBoards) {
                cout << "In getBoard() inside AllBoards class, boardIndex "
                     << boardIndex << " is out of range. Exiting..." << endl;
                exit( -1);
            }
            
            // Construct a Board from one of all the boards
            Board newBoard( boardIndex+1, allBoards[ boardIndex], getParValue( boardIndex) );
            return newBoard;
        }
    
        //------------------------------------------------------------------------------------------------------------------
        // Retrieve the par value
        int getParValue( int boardIndex) {
            int theParValue = 15;   // default
            if( boardIndex < NumberOfPresetBoardParValues) {
                theParValue = levelParValues[ boardIndex];
            }
            return theParValue;
        } // end getParValue()
    
    private:
        int par = 0;             // The number of moves it should take to solve this level
        int currentLevel = -1;   // Which board we're on
        int allBoards[ 118][ BoardRows * BoardColumns];   // Array to store all 118 8x10 boards
    
        // Par values for levels 0 through 59.  Default is 15 after that.
        const int levelParValues[ NumberOfPresetBoardParValues] =
           // 0   1   2   3   4   5   6   7   8   9
            { 4,  3, 14,  5,  6,  4,  4, 14,  5, 25,  //  0 -  9
             12, 17, 17,  8,  4, 12,  9,  8, 12,  8,  // 10 - 19
             10, 10, 16, 13, 20, 14, 10,  9, 12, 14,  // 20 - 29
             15, 13, 20,  8, 15, 10, 10, 11,  7, 23,  // 30 - 39
              8, 11, 16, 14, 12, 13, 13,  3, 35, 18,  // 40 - 49
             26, 10, 13, 18, 26, 12, 15,  5, 22, 15}; // 50 - 59
}; //end class allBoards


//------------------------------------------------------------------------------------------------------------------
// AllBoards constructor
AllBoards::AllBoards()
 {
    // Read in all the boards from the data file
    ifstream inputFileStream;  // declare the input file stream

    // Open input file and verify file open worked.
    inputFileStream.open( FileName);
    if( !inputFileStream.is_open()) {
        cout << "Could not find input file. " << FileName << ".  Exiting..." << endl;
        exit( -1);
    }

    // Read the five lines of comments at the top of the datafile.  Datafile structure is:
    //    // Originally from Vexed v2.0 - globals.c "Global variable declarations"
    //    // Copyright (C) 1999 James McCombe (cybertube@earthling.net)
    //    // September 1,2001 - Version 2.0 changes by Mark Ingebretson (ingebret@yahoo.com) and others.
    //    // Oct 13, 2019  Format changed by Dale Reed (reed @ uic.edu)
    //    // Covered by the GNU General Public License https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html
    //    // Level 000
    //    9 9 9 9 9 9 9 9 9 9
    //    9 9 9 9 9 9 9 9 9 9
    //    9 9 9 2 1 0 0 9 9 9
    //    9 9 9 9 9 0 0 9 9 9
    //    9 9 9 0 0 0 0 9 9 9
    //    9 9 9 1 0 0 2 9 9 9
    //    9 9 9 9 1 2 9 9 9 9
    //    9 9 9 9 9 9 9 9 9 9
    //    // Level 001
    //    9 9 9 9 9 9 9 9 9 9
    //    9 9 9 9 9 9 9 9 9 9
    // ...
    
    // Read and discard the five lines of comments at the beginning of the file.
    string inputLine;
    for( int i=0; i<5; i++) {
        getline(inputFileStream, inputLine);
    }
    
    // Now read each of the 118 (MaxNumberOfBoards) boards.  Each one starts with a comment that has the board number.
    for( int boardIndex = 0; boardIndex < MaxNumberOfBoards; boardIndex++) {
        // Read the comment with the board number
        getline(inputFileStream, inputLine);
        
        // For each board now read the BoardRows * BoardColumns number of board values, as integers
        for( int row=0; row<BoardRows; row++) {
            for( int col=0; col<BoardColumns; col++) {
                inputFileStream >> allBoards[ boardIndex][ row * BoardColumns + col];
            }
            // Get rid of end-of-line character at the end of each board, so we can get a clean start on reading the
            //   first line of the next board, on the line with the comment and board number.
            getline(inputFileStream, inputLine);
        }
    }//end for( int boardIndex=0...

    // Close the input file.
    inputFileStream.close();
}//end AllBoards() constructor


//------------------------------------------------------------------------------------------------------------------
// Clear the initial screen and give game instructions.  Depending on user input, set the global variable
// used to determine whether or not to clear the screen between displays.  Running interatively works
// better with clearing the screen, however running the automated assessments requires not clearing the screen.
void displayDirections( )
{
    cout << "Welcome to Vexed!  \n"
         << "The objective is to place identical pieces next to each other, so that they vanish,   \n"
         << "clearing the board completely within the indicated number of moves. On each move      \n"
         << "enter the row, column, and direction (L or R) to move, in either upper or lower       \n"
         << "case. You may also enter 'x' to exit the program, 'r' to reset the current level,     \n"
         << "or 'u' to undo the most recent move.     \n"
         << endl;
    
    char clearTheBoard;
    cout << "Clear the board between displays (y/n) ? ";
    cin >> clearTheBoard;
    clearTheBoard = toupper( clearTheBoard);
    
    if( clearTheBoard == 'Y') {
        UseSystemClear = true;   // Default is false
    }
}


//-------------------------------------------------------------------------------------
// Node declaration to implement a linked list to store moves, used to implement
// undo within a level. You should store the old board, the old score, the old boardIndex,
// and a pointer to the next Node.
struct Node{
  int moveNumber; 
  int score;
  int currentBoardIndex;
  Board theBoard;
  Node* pNext;
};


//--------------------------------------------------------------------------------
// Display the move numbers on the linked list
void displayList( Node *pTemp) /// TS--transverse node
{
  cout<<"   Score: "<<pTemp->score<<", List of board:moveNumber is: ";
  while(pTemp != NULL){

    cout<<pTemp->currentBoardIndex<<":"<<pTemp->theBoard.getMoveNumber();

    
} // end displayList()


//--------------------------------------------------------------------------------
// Delete the front node on the list and restore current game values from the
// next node that reflects the previous move.
// Parameters should be:
//    pointer to the head of the list, which could change, so should be a Node * reference.
//    the Board, which should also be a reference parameter, and is the game board to be restored from the list
//    the score, which should be restored from the list
//    the currentBoardIndex, which should be restored from the list
void deleteNodeFromList(int &currentBoardIndex, int &score, Board &theBoard, Node * &pHead, int &moveNumber)
{
    // ...
    // Error check if we can't undo
    if( pHead->pNext == NULL) {
        cout << "*** You cannot undo past the beginning of the game.  Please retry. ***" << endl;
        return;
    }else{
      cout<<"* Undoing move * ";
      // Keep track of old head of list
      Node *pTemp = pHead;
      // Advance head pointer and delete old list head
      pHead = pHead->pNext;
      delete pTemp;
      // Use new list head info to restore game play values
      currentBoardIndex = pHead->currentBoardIndex;
      moveNumber = pHead->moveNumber;
      score = pHead->score;
      theBoard = pHead->theBoard;
      theBoard.displayBoard();
    }
} //end deleteNodeFromList()


//--------------------------------------------------------------------------------
// Create a new node and prepend it to the beginning of the list.
// Parameters should be:
//    pointer to the head of the list, which could change, so should be a Node * reference.
//    the Board, which should also be a reference parameter, and is the game board to be restored from the list
//    the score, which should be restored from the list
//    the currentBoardIndex, which should be restored from the list
void addNodeToList(int currentBoardIndex, int score, Board &theBoard, Node * &pHead, int moveNumber)
{
    // Create a new node and store current values into it
    // ...
    Node *pTemp = new Node;
    pTemp->currentBoardIndex = currentBoardIndex;
    pTemp->score = score;
    pTemp->moveNumber = moveNumber;
    pTemp->theBoard = theBoard;
    
    // Prepend it onto the front of the list
    // ...
    pTemp->pNext = pHead;
    pHead = pTemp;
}


//------------------------------------------------------------------------------------------------------------------
// Driver for the program, using the classes and functions declared above
int main()
{
    AllBoards allTheBoards;     // Reads in and stores all the boards from a data file
    int currentBoardIndex = 1;  // Starting board index (Gets translated to 0-based indexing in the getBoard() function.)
    Board theBoard;             // The board instance, that is set to hold the values for each level
    int score = 0;              // Score accumulates par points for each level finished
    char moveAction = ' ';      // This is set to 'X' for exit, 'R' for reset, and 'S' to set the level
    int moveNumber =0;
		// Declare pHead to be a Node pointer, and initialize it to NULL
    // ...
    Node *pHead = NULL;// head of linked lists

    // Create the initial board and store it on the list.  The list may grow and shrink, but
    //    this initial board should always be on the list.
    theBoard = allTheBoards.getBoard( currentBoardIndex);
    // Add a new Node with theBoard onto the list
    Node *pTemp; // used in the new node creation
    displayDirections();
    bool undoDone = false;
  
  
    // Infinite loop to play the game
    while( true) {
        // Display the current board
        theBoard.displayBoard();
        moveNumber =0;
        
        // Make moves to try and solve the puzzle of the current level
        while( ! theBoard.boardIsClear() ) {
            // Display the linked list
            // ...
            if (undoDone == false){
            
              addNodeToList(currentBoardIndex, score,theBoard ,pHead, moveNumber++);
            }else{
              undoDone = false;

            }
            displayList(pHead); //Displays the linked list          

            // Get the user input and make a move
            moveAction = ' ';   // Will get reset below for user selections to exit, reset, or select level.
            theBoard.getUserInputAndMakeMove( moveAction);

            // If userinput indicates exit, reset level, or set a new level, then break out of this
            //    level's playing loop to handle those.
            if( moveAction == 'X' || moveAction == 'R' || moveAction == 'S' ) {
                break; 
            }
            
            // Handle userinput of 'U' to undo a move
            if( moveAction == 'U') {
                // Ensure there is more than just the original node on the list before allowing undo.
                // If there is only the original node on the list and undo is selected, give the error message:
                // "*** You cannot undo past the beginning of the game.  Please retry. ***"
 
								// If there is more than one node on the list, delete the front node and advance the list head pointer,
								// then display the current board
								// ...
								deleteNodeFromList(currentBoardIndex, score, theBoard, pHead, moveNumber);
                undoDone = true;

              continue;   // Go back up to top of loop to display board and prompt for next move
            }
            
            // Keep doing the following two steps until the board no longer changes
            Board boardCopy;
            do {
                boardCopy = theBoard;     // Make a copy of the current board using the copy constructor
                
                 theBoard.slideAllPiecesDown();             // Slide all possible pieces down
                theBoard.findAndDeleteMatchingPieces();    // Delete any adjacent matching pieces
            } while( theBoard.isNotTheSameAs( boardCopy) );

            // Prepend board, score and currentBoardIndex to a new node at the front of the list.
            // Don't add empty boards that exist at the end of a move.  If empty boards are allowed to be
            // added, then undoing to the end of the previous level always takes the user to an empty
            // board, which then automatically advances to the beginning of the next level.
            // .
                    
        } // end while( !...
        
        // Handle the special moveAction cases for exit, reset, and selecting the level
        if( moveAction == 'X') {
            // User input was 'X' to exit the game
            exit( 0);
        }
        else if( moveAction == 'R') {
            // User input was 'R' to reset the level
            continue;   // Jump back up to top of loop to reset the board at the current level
        }
        else if(  moveAction == 'S') {
            // User input was 'S' to select the level for play.  This should only be used for testing,
            // since score is no longer meaningful once you jump ahead a level.
            cin >> currentBoardIndex;
            continue;   // Jump back up to top of loop to select this board
        }
        
        // Advance to the next level only if this level was solved within par moves
        if( theBoard.boardClearedWithinParMoves() ) {
            cout << endl
                 << "Congratulations!  On to the next level." << endl;
            currentBoardIndex++;
            score = score + theBoard.getParValue();
            cout << "Score: " << score << endl;
            
            // Pause the program for designated amount of time
            std::this_thread::sleep_for(std::chrono::milliseconds( ExtraSleepAmountAfterCompletingLevel));

        }
        else {
            cout << "Sorry, you took " << theBoard.getMoveNumber()
                 << " moves and you must finish within "
                 << theBoard.getParValue() << " moves before moving on. " << endl;
        }
        
        // Get the current board, which may be reseting the current level, setting a new board level,
        // or the next board after completing the existing level.
        theBoard = allTheBoards.getBoard( currentBoardIndex);
        // Add the new board to the list
        // ...
        
    } //end while( true)
    
    return 0;
}
