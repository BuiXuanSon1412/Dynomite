#ifndef SCREEN1VIEW_HPP
#define SCREEN1VIEW_HPP

#include <gui_generated/screen1_screen/Screen1ViewBase.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>
//#include <touchgfx/widgets/canvas/CanvasLine.hpp>
#include <images/BitmapDatabase.hpp>

#define NUM_ROWS 10
#define NUM_COLS 9

#define EGG_WIDTH 26
#define EGG_HEIGHT 29
#define EGG_BLANK_SIZE 8

#define SCREEN_WIDTH 240
#define PADDING 3

#define LIMIT_Y 270
#define BASE_Y 295

#define MAX_LEN 90

struct Vec2 {
	float x, y;
};

struct Index {
	int rowIndex, colIndex;
};

class IndexQueue {
private:
	Index q[MAX_LEN];
	int head = 0, tail = 0;
public:
	Index front();
	bool pop();
	bool push(Index index);
	bool empty();
	bool full();
};
enum EggState {
	IDLE,
	READY,
	AIRBORNE,
};

class Screen1View : public Screen1ViewBase
{
public:
    Screen1View();
    virtual ~Screen1View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    void handleTickEvent();
    void onPlayButtonClicked();

protected:
    // varibles to control game flow
    const int framePerSecond = 30;
    float dEggBatchY;
    bool gameState;
    uint32_t lastUpdateTickCount;

    // seed for random generator
    uint32_t seed;

    // variables to manage the batch of eggs
    touchgfx::Image eggBatch[NUM_ROWS][NUM_COLS];
    bool eggBatchState[NUM_ROWS][NUM_COLS];
    uint16_t eggBatchBitmapID[NUM_ROWS][NUM_COLS];
    int eggBatchDegree[NUM_ROWS][NUM_COLS];
    float eggBatchY[NUM_ROWS];
    int startRowIndex;

    // variable to manage the shooting egg and the next shooting egg
    touchgfx::Image shootingEgg, nextShootingEgg;
    EggState shootingEggState;
    uint16_t shootingEggBitmapID, nextShootingEggBitmapID;
    float shootingEggX, shootingEggY;
    float dShootingEggX, dShootingEggY, dShootingEgg;
    int leftToRight;
    int shootingLineEndX, shootingLineEndY;

    // array of eggs

    const uint16_t eggBitmapID[6] = {
    		BITMAP_BLUE_EGG_ID,
			BITMAP_GREEN_EGG_ID,
			BITMAP_GREENRED_EGG_ID,
			BITMAP_GREY_EGG_ID,
			BITMAP_ORANGE_EGG_ID,
			BITMAP_WHITE_EGG_ID
    };
    int eggBitmapIDRange;

    // step to reach neighbors of odd-indexed row egg and even-indexed row egg
    const int stepsForEvenRowIndex[6][2] = {{0, -1}, {0, 1}, {-1, 0}, {-1, 1}, {1, 0}, {1, 1}};
    const int stepsForOddRowIndex[6][2] = {{0, -1}, {0, 1}, {-1, -1}, {-1, 0}, {1, -1}, {1, 0}};

    //const int steps[2][6][2] = {{{0, -1}, {0, 1}, {-1, 0}, {-1, 1}, {1, 0}, {1, 1}},
    //		{{0, -1}, {0, 1}, {-1, -1}, {-1, 0}, {1, -1}, {1, 0}}};
private:
    void initializeEggBatch();
    void updateEggBatch();
    void renderEggBatch();

    void initializeShootingEgg();
    void updateShootingEgg();
    void renderShootingEgg();

    void initializeNextShootingEgg();
    void updateNextShootingEgg();
    void renderNextShootingEgg();

    void initializeShootingLine();
    void updateShootingLine();
    void renderShootingLine();

    Index detectCollisionBetweenShootingEggAndEggBatch();
    bool checkCollisionArea(Vec2 p, Vec2 v1, Vec2 v2, Vec2 v3);
    float sign(Vec2 v1, Vec2 v2, Vec2 v3);

	void updateEggBatchAfterCollision(Index shootingEggIndex);
    void updateShootingEggAfterCollision();
    void updateNextShootingEggAfterCollision();

    uint16_t generateRandomEggBitmapID();
    uint32_t lcd_rand();
};

#endif // SCREEN1VIEW_HPP
