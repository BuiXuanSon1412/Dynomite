#ifndef SCREEN1VIEW_HPP
#define SCREEN1VIEW_HPP

#include <gui_generated/screen1_screen/Screen1ViewBase.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>

#include <images/BitmapDatabase.hpp>

#define NUM_ROWS 10
#define NUM_COLS 9
#define EGG_WIDTH 26
#define EGG_HEIGHT 29

class Screen1View : public Screen1ViewBase
{
public:
    Screen1View();
    virtual ~Screen1View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    void handleTickEvent();
    //void onPlayButtonClicked();

protected:
    // varibles to control game flow
    const int framePerSecond = 30;
    const int step = 1;
    int framePerEggBatchUpdate;			// smaller -> harder
    int frameCountForEggBatchUpdate;
    uint32_t lastUpdateTickCount;

    // variables to manage the batch of eggs
    touchgfx::Image eggBatch[NUM_ROWS][NUM_COLS];
    bool eggBatchState[NUM_ROWS][NUM_COLS];
    int eggBatchY[NUM_ROWS];
    int startRowIndex;

    // variable to manage the shooting egg and the next shooting egg
    touchgfx::Image shootingEgg, nextShootingEgg;

    // array of eggs
    uint32_t seed;
    const touchgfx::Bitmap eggBitmaps[6] = {
    		touchgfx::Bitmap(BITMAP_BLUE_EGG_ID),
			touchgfx::Bitmap(BITMAP_GREEN_EGG_ID),
			touchgfx::Bitmap(BITMAP_GREENRED_EGG_ID),
			touchgfx::Bitmap(BITMAP_GREY_EGG_ID),
			touchgfx::Bitmap(BITMAP_ORANGE_EGG_ID),
			touchgfx::Bitmap(BITMAP_WHITE_EGG_ID),
    };
    int eggBitmapIndexRange;
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

    touchgfx::Bitmap generateRandomEggBitmap();
    uint32_t lcd_rand();
};

#endif // SCREEN1VIEW_HPP
