#ifndef SCREEN1VIEW_HPP
#define SCREEN1VIEW_HPP

#include <gui_generated/screen1_screen/Screen1ViewBase.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>

#include <images/BitmapDatabase.hpp>

#define NUM_ROWS 10
#define NUM_COLS 9
#define EGG_WIDTH 26
#define EGG_HEIGHT 29
#define PADDING 3

struct Vec2 {
	float x, y;
};

enum EggState {
	IDLE,
	READY,
	AIRBORNE,
	COLLIDED
};
struct Index {
	int rowIndex, colIndex;
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
    const int step = 1;
    int framePerEggBatchUpdate;			// smaller -> harder
    int frameCountForEggBatchUpdate;
    uint32_t lastUpdateTickCount;

    // seed for random generator
    uint32_t seed;

    // variables to manage the batch of eggs
    touchgfx::Image eggBatch[NUM_ROWS][NUM_COLS];
    bool eggBatchState[NUM_ROWS][NUM_COLS];
    int eggBatchY[NUM_ROWS];
    int startRowIndex;

    // variable to manage the shooting egg and the next shooting egg
    touchgfx::Image shootingEgg, nextShootingEgg;
    EggState shootingEggState;

    float shootingEggX, shootingEggY;
    float dShootingEggX, dShootingEggY;
    // array of eggs
    const touchgfx::Bitmap eggBitmaps[6] = {
    		touchgfx::Bitmap(BITMAP_BLUE_EGG_ID),
			touchgfx::Bitmap(BITMAP_GREEN_EGG_ID),
			touchgfx::Bitmap(BITMAP_GREENRED_EGG_ID),
			touchgfx::Bitmap(BITMAP_GREY_EGG_ID),
			touchgfx::Bitmap(BITMAP_ORANGE_EGG_ID),
			touchgfx::Bitmap(BITMAP_WHITE_EGG_ID),
    };
    int eggBitmapIndexRange;

    // step to reach neighbors of odd-indexed row egg and even-indexed row egg
    const int stepsForEvenRowIndex[6][2] = {{0, -1}, {0, 1}, {-1, 0}, {-1, 1}, {1, 0}, {1, 1}};
    const int stepsForOddRowIndex[6][2] = {{0, -1}, {0, 1}, {-1, -1}, {-1, 0}, {1, -1}, {1, 0}};

    // limit line
    const int limitY = 250;
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

    Index detectCollisionBetweenShootingEggAndEggBatch();
    bool checkCollisionArea(Vec2 p, Vec2 v1, Vec2 v2, Vec2 v3);
    float sign(Vec2 v1, Vec2 v2, Vec2 v3);

	void updateEggBatchAfterCollision(Index shootingEggIndex);
    void updateShootingEggAfterCollision();
    void updateNextShootingEggAfterCollision();

    touchgfx::Bitmap generateRandomEggBitmap();
    uint32_t lcd_rand();
};

#endif // SCREEN1VIEW_HPP
