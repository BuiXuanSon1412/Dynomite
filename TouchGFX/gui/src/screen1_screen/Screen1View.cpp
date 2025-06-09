#include <gui/screen1_screen/Screen1View.hpp>

#include <cmsis_os.h>

Screen1View::Screen1View()
{
	startRowIndex = 4;
	framePerEggBatchUpdate = 10;
	frameCountForEggBatchUpdate = 0;
	eggBitmapIndexRange = 3;
	seed = 123456789;
}

void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();
    initializeEggBatch();
    //renderEggBatch();

    initializeShootingEgg();
    initializeNextShootingEgg();

    playButton.setVisible(false);

    lastUpdateTickCount = osKernelGetTickCount();
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}

void Screen1View::handleTickEvent() {
	Screen1ViewBase::handleTickEvent();
	uint32_t tickPerFrame = osKernelGetTickFreq() / framePerSecond;

	if (osKernelGetTickCount() - lastUpdateTickCount > tickPerFrame) {
		if (eggBatchY[startRowIndex] >= 200) {
			playButton.setVisible(true);
			return;
		}

		if (frameCountForEggBatchUpdate++ == framePerEggBatchUpdate) {
			updateEggBatch();
			renderEggBatch();
			frameCountForEggBatchUpdate = 0;
		}
		updateShootingEgg();
		renderShootingEgg();
		lastUpdateTickCount = osKernelGetTickCount();
	}
}

void Screen1View::initializeEggBatch() {
	int i = startRowIndex;
	int lastY = -1;
	do {
		// Y coordinate
		if (lastY == -1) eggBatchY[i] = startRowIndex * EGG_HEIGHT + 3;
		else eggBatchY[i] = lastY - EGG_HEIGHT;
		lastY = eggBatchY[i];

		// set XY
		int numOfCols = NUM_COLS;
		int leftPadding = 3;
		if (i % 2 == 0) {
			leftPadding = 16;
			numOfCols--;
		}
		for (int j = 0; j < numOfCols; j++) {
			eggBatch[i][j].setBitmap(generateRandomEggBitmap());
			eggBatch[i][j].setXY(leftPadding + EGG_WIDTH * j, eggBatchY[i]);
			eggBatchState[i][j] = true;
			add(eggBatch[i][j]);
		}
		i = i-1;
		if (i < 0) i += NUM_ROWS;
	} while (i != startRowIndex);
}

void Screen1View::renderEggBatch() {
	for (int i = 0; i < NUM_ROWS; i++) {
		if (eggBatchY[i] + 0.5 * EGG_HEIGHT <= 0) break;
		int numOfCols = (i%2 == 0 ? NUM_COLS-1 : NUM_COLS);
		for (int j = 0; j < numOfCols; j++) {
			if (eggBatchState[i][j]) {
				eggBatch[i][j].invalidate();
			}
		}
	}
}

void Screen1View::updateEggBatch() {
	// update Y coordinate of all rows in 'eggBatch'
	for (int i = 0; i < NUM_ROWS; i++) {
		eggBatchY[i]++;
		int numOfCols = (i%2 == 0 ? NUM_COLS-1 : NUM_COLS);
		for (int j = 0; j < numOfCols; j++) {
			eggBatch[i][j].moveTo(eggBatch[i][j].getX(), eggBatchY[i]);
		}
	}

	//if (checkIfShootingEggIsCollided()) { // this validator also update 'shootingEgg' into 'eggBatch'
		// detect which eggs shooting egg can drop and update 'eggBatchState'
	//}


	int i = startRowIndex;
	do {
		// number of eggs in a specific row
		int numOfCols = (i%2 == 0 ? NUM_COLS-1 : NUM_COLS);

		// update 'startRowIndex'
		if (eggBatchY[i] + 0.5 * EGG_HEIGHT > 0) {
			bool empty = true;
			for (int j = 0; j < numOfCols; j++) {
				if (eggBatchState[i][j]) empty = false;
			}
			if (empty) {
				startRowIndex--;
				if (startRowIndex < 0) startRowIndex += NUM_ROWS;
			}
		}
		// prepare eggs which do not show up on screen
		else {
			for (int j = 0; j < numOfCols; j++) {
				if (eggBatchState[i][j] == false) {
					eggBatch[i][j].setBitmap(generateRandomEggBitmap());
					eggBatchState[i][j] = true;
				}
			}
		}
		i = i-1;
		if (i < 0) i += NUM_ROWS;
	} while (i != startRowIndex);
}


void Screen1View::initializeShootingEgg() {
	shootingEgg.setBitmap(generateRandomEggBitmap());
	shootingEgg.setXY(107, 280);
	add(shootingEgg);
}
void Screen1View::updateShootingEgg() {
	//sampleEgg.moveTo(sampleEgg.getX(), sampleEgg.getY() + 2);
}

void Screen1View::renderShootingEgg() {
	shootingEgg.invalidate();
}

void Screen1View::initializeNextShootingEgg() {
	nextShootingEgg.setBitmap(generateRandomEggBitmap());
	nextShootingEgg.setXY(10, 280);
	add(nextShootingEgg);
}
void Screen1View::updateNextShootingEgg() {
	//sampleEgg.moveTo(sampleEgg.getX(), sampleEgg.getY() + 2);
}

void Screen1View::renderNextShootingEgg() {
	nextShootingEgg.invalidate();
}

uint32_t Screen1View::lcd_rand() {
	seed = (1103515245 * seed + 12345) & 0x7fffffff;
	return seed;
}
touchgfx::Bitmap Screen1View::generateRandomEggBitmap() {
	uint32_t randomEggBitmapIndex = lcd_rand() % eggBitmapIndexRange;
	return eggBitmaps[randomEggBitmapIndex];
}
