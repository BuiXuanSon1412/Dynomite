#include <gui/screen1_screen/Screen1View.hpp>

#include <images/BitmapDatabase.hpp>


Screen1View::Screen1View()
{

}

void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();
    renderEggImages();
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}

void Screen1View::renderEggImages() {
	for (int i = 0; i < NUM_ROWS; i++) {
		for (int j = 0; j < NUM_COLS; j++) {
			eggs[i][j].setBitmap(touchgfx::Bitmap(BITMAP_BLUE_EGG_ID));
			eggs[i][j].setXY(3+26*i, 3+29*j);
			add(eggs[i][j]);
		}
	}
}
