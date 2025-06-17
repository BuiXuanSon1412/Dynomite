#include <gui/screen1_screen/Screen1View.hpp>

#include <cmsis_os.h>
#include <cmath>
#include <queue>
#include <utility>
#include <cstring>
#include <vector>

extern uint16_t controllerX, controllerY;
extern uint16_t prevControllerX, prevControllerY;
extern uint16_t currentScore;
extern uint16_t highScore;

Screen1View::Screen1View()
{
	startRowIndex = 4;
	gameState = false;
	dEggBatchY = 0.03;

	eggBitmapIDRange = 3;
	seed = 123456789;

	shootingEggX = 107;
	shootingEggY = 280;
	dShootingEgg = 2;
	shootingEggState = IDLE;
}

void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}

void Screen1View::onPlayButtonClicked() {
	scoreContainer.setVisible(false);
	scoreContainer.invalidate();
	gameState = true;
	initializeEggBatch();
	initializeShootingEgg();
	initializeNextShootingEgg();
	lastUpdateTickCount = osKernelGetTickCount();
}

void Screen1View::handleTickEvent() {
	Screen1ViewBase::handleTickEvent();
	uint32_t tickPerFrame = osKernelGetTickFreq() / framePerSecond;

	if (osKernelGetTickCount() - lastUpdateTickCount > tickPerFrame) {
		if (gameState == false) {
			scoreContainer.setVisible(true);
			scoreContainer.invalidate();
			return;
		}
		if (eggBatchY[startRowIndex] + 0.5 * EGG_HEIGHT > limitY) {
			gameState = false;
			return;
		}
		updateEggBatch();
		updateShootingEgg();
		//updateNextShootingEgg();

		if (shootingEggState == AIRBORNE) {
			Index shootingEggIndex = detectCollisionBetweenShootingEggAndEggBatch();
			// detect which eggs shooting egg can drop and update 'eggBatchState'
			if (shootingEggIndex.rowIndex != -1 && shootingEggIndex.colIndex != -1) {
				updateEggBatchAfterCollision(shootingEggIndex);
				updateShootingEggAfterCollision();
				updateNextShootingEggAfterCollision();
			}
		}

		renderEggBatch();
		renderShootingEgg();
		renderNextShootingEgg();

		lastUpdateTickCount = osKernelGetTickCount();
	}

}

void Screen1View::initializeEggBatch() {
	int i = startRowIndex;
	int lastY = -1;
	do {
		// Y coordinate
		if (lastY == -1) eggBatchY[i] = startRowIndex * EGG_HEIGHT;
		else eggBatchY[i] = lastY - EGG_HEIGHT;
		lastY = eggBatchY[i];

		// set XY, bitmap and state
		// adding to View
		int numOfCols, padding;

		if (i % 2 == 0) {
			padding = PADDING + EGG_WIDTH * 0.5;
			numOfCols = NUM_COLS - 1;
		}
		else {
			padding = PADDING;
			numOfCols = NUM_COLS;
		}
		for (int j = 0; j < numOfCols; j++) {
			eggBatchBitmapID[i][j] = generateRandomEggBitmapID();
			eggBatch[i][j].setBitmap(touchgfx::Bitmap(eggBatchBitmapID[i][j]));
			eggBatch[i][j].setXY(padding + EGG_WIDTH * j, (int)eggBatchY[i]);
			eggBatchState[i][j] = true;
			add(eggBatch[i][j]);
		}
		i = i-1;
		if (i < 0) i += NUM_ROWS;
	} while (i != startRowIndex);
}

void Screen1View::renderEggBatch() {
	for (int i = 0; i < NUM_ROWS; i++) {
		//if (eggBatchY[i] + 0.5 * EGG_HEIGHT <= 0) break;
		int numOfCols = (i%2 == 0 ? NUM_COLS-1 : NUM_COLS);
		for (int j = 0; j < numOfCols; j++) {
			if (eggBatchState[i][j]) {
				eggBatch[i][j].moveTo(eggBatch[i][j].getX(), (int)eggBatchY[i]);
				eggBatch[i][j].invalidate();
			}
		}
	}
}

void Screen1View::updateEggBatch() {
	// update Y coordinate of all rows in 'eggBatch'
	for (int i = 0; i < NUM_ROWS; i++) {
		eggBatchY[i] += dEggBatchY;
	}
}


void Screen1View::initializeShootingEgg() {
	shootingEggBitmapID = generateRandomEggBitmapID();
	shootingEgg.setBitmap(touchgfx::Bitmap(shootingEggBitmapID));
	shootingEgg.setXY((int)std::round(shootingEggX), (int)std::round(shootingEggY));
	add(shootingEgg);
}
void Screen1View::updateShootingEgg() {
	switch (shootingEggState) {
	case IDLE:
		if (controllerY <= 135) return;
		else {
			shootingEggState = READY;
		}
		break;
	case READY:
		if (controllerY <= 135 && controllerY >= 120
				&& controllerX <= 135 && controllerX >= 120) {
			shootingEggState = AIRBORNE;
			float dx = prevControllerX - 127.5f;
			float dy = prevControllerY - 127.5f;
			dShootingEggX = -dx / sqrt(dx * dx + dy * dy) * dShootingEgg;
			dShootingEggY = -dy / sqrt(dx * dx + dy * dy) * dShootingEgg;
		}
		break;
	case AIRBORNE:
		if (shootingEggX + 0.5 * EGG_WIDTH >= SCREEN_WIDTH
			|| shootingEggX - 0.5 * EGG_WIDTH <= 0) {
			dShootingEggX = -dShootingEggX;
		}
		shootingEggX += dShootingEggX;
		shootingEggY += dShootingEggY;
		break;
	default:
		break;
	}

}

void Screen1View::renderShootingEgg() {
	shootingEgg.moveTo((int)std::round(shootingEggX), (int)std::round(shootingEggY));
	shootingEgg.invalidate();
}

void Screen1View::initializeNextShootingEgg() {
	nextShootingEggBitmapID = generateRandomEggBitmapID();
	nextShootingEgg.setBitmap(touchgfx::Bitmap(nextShootingEggBitmapID));
	nextShootingEgg.setXY(10, 280);
	add(nextShootingEgg);
}
void Screen1View::updateNextShootingEgg() {
	//sampleEgg.moveTo(sampleEgg.getX(), sampleEgg.getY() + 2);
}

void Screen1View::renderNextShootingEgg() {
	nextShootingEgg.invalidate();
}

Index Screen1View::detectCollisionBetweenShootingEggAndEggBatch() {
	if (shootingEggY - 0.5 * EGG_HEIGHT > eggBatchY[startRowIndex] + 0.5 * EGG_HEIGHT) return {-1, -1};
	int i = startRowIndex;

	do {
		Vec2 p, v1, v2, v3;
		int numOfCols = (i%2 == 0 ? NUM_COLS-1 : NUM_COLS);
		//if (eggBatchY[i] + 0.5 * EGG_HEIGHT <= 0) continue;
		for (int j = 0; j < numOfCols; j++) {
			if (eggBatchState[i][j]) {
				// bottom-right of eggBatch[i][j]
				p = {shootingEggX, shootingEggY - 0.5f * EGG_HEIGHT};
				v1 = {(float) eggBatch[i][j].getX(), eggBatchY[i]};
				v2 = {eggBatch[i][j].getX(), eggBatchY[i] + 0.5f * EGG_HEIGHT};
				v3 = {eggBatch[i][j].getX() + 0.5f * EGG_WIDTH, eggBatchY[i] + 0.5f * EGG_HEIGHT};
				if (checkCollisionArea(p, v1, v2, v3)){
					int rowIndex, colIndex;
					rowIndex = (i+1) % NUM_ROWS;
					if (i % 2 == 0) colIndex = j+1;
					else if (j == numOfCols-1) colIndex = j-1;
					else colIndex = j;
					return {rowIndex, colIndex};
				}
				// right side of eggBatch[i][j]
				p = {shootingEggX - 0.5f * EGG_WIDTH, shootingEggY - 0.5f * EGG_HEIGHT};
				v1 = {(float) eggBatch[i][j].getX(), (float) eggBatchY[i]};
				v2 = {eggBatch[i][j].getX() + 0.5f * EGG_WIDTH, eggBatchY[i] + 0.5f * EGG_HEIGHT};
				v2 = {eggBatch[i][j].getX() + 0.5f * EGG_WIDTH, eggBatchY[i] - 0.5f * EGG_HEIGHT};

				if (checkCollisionArea(p, v1, v2, v3)) {
					int rowIndex, colIndex;

					if (j == numOfCols-1) {
						rowIndex = (i+1) % NUM_ROWS;
						if (i % 2 == 0) colIndex = j+1;
						else colIndex = j-1;
					}
					else {
						rowIndex = i;
						colIndex = j+1;
					}
					return {rowIndex, colIndex};
				}


				// bottom-left of eggBatch[i][j]
				p = {shootingEggX, shootingEggY - 0.5f * EGG_HEIGHT};
				v1 = {(float) eggBatch[i][j].getX(), eggBatchY[i]};
				v2 = {eggBatch[i][j].getX(), eggBatchY[i] + 0.5f * EGG_HEIGHT};
				v3 = {eggBatch[i][j].getX() - 0.5f * EGG_WIDTH, eggBatchY[i] + 0.5f * EGG_HEIGHT};
				if (checkCollisionArea(p, v1, v2, v3)) {
					int rowIndex, colIndex;
					rowIndex = (i+1) % NUM_ROWS;
					if (i % 2 == 0) colIndex = j;
					else if (j == 0) colIndex = j;
					else colIndex = j-1;
					return {rowIndex, colIndex};
				}

				// left side of eggBatch[i][j]
				p = {shootingEggX + 0.5f * EGG_WIDTH, shootingEggY - 0.5f * EGG_HEIGHT};
				v1 = {(float) eggBatch[i][j].getX(), eggBatchY[i]};
				v2 = {eggBatch[i][j].getX() - 0.5f * EGG_WIDTH, eggBatchY[i] + 0.5f * EGG_HEIGHT};
				v2 = {eggBatch[i][j].getX() - 0.5f * EGG_WIDTH, eggBatchY[i] - 0.5f * EGG_HEIGHT};
				if (checkCollisionArea(p, v1, v2, v3)) {
					int rowIndex, colIndex;
					if (j == 0) {
						rowIndex = (i+1) % NUM_ROWS;
						colIndex = 0;
					} else {
						rowIndex = i;
						colIndex = j-1;
					}
					return {rowIndex, colIndex};
				}
			}
		}
		i = i-1;
		if (i < 0) i += NUM_ROWS;
	} while (i != startRowIndex);
	return {-1, -1};
}
float Screen1View::sign(Vec2 v1, Vec2 v2, Vec2 v3) {
    return (v1.x - v3.x) * (v2.y - v3.y) -
           (v2.x - v3.x) * (v1.y - v3.y);
}
bool Screen1View::checkCollisionArea(Vec2 p, Vec2 v1, Vec2 v2, Vec2 v3) {
	bool b1 = sign(p, v1, v2) < 0.0f;
	bool b2 = sign(p, v2, v3) < 0.0f;
	bool b3 = sign(p, v3, v1) < 0.0f;

	return ((b1 == b2) && (b2 == b3));
}

void Screen1View::updateEggBatchAfterCollision(Index shootingEggIndex) {
	// temporary update shootingEgg on eggBatch
	int rowIndex = shootingEggIndex.rowIndex;
	int colIndex = shootingEggIndex.colIndex;

	eggBatch[rowIndex][colIndex].setBitmap(touchgfx::Bitmap(shootingEggBitmapID));
	eggBatchState[rowIndex][colIndex] = true;

	// if shootingEgg landed on the new row of eggBatch, update ...
	if (rowIndex == (startRowIndex+1) % NUM_ROWS) {
		int numOfCols = (rowIndex % 2 == 0 ? NUM_COLS-1 : NUM_COLS);
		for (int j = 0; j < numOfCols; j++) {
			if (j != colIndex) {
				eggBatchState[rowIndex][j] = false;
			}
		}
		eggBatchY[rowIndex] = eggBatchY[startRowIndex] + EGG_HEIGHT;
		startRowIndex = rowIndex;
	}

	// detect which eggs of eggBatch drop:
	// using BFS for simplicity to reach to all eggBatch 's elements with the color of shootingEgg


	IndexQueue Q = IndexQueue();

	Index eggIndices[MAX_LEN];
	int eggCount = 0;
	bool visited[NUM_ROWS][NUM_COLS];
	memset(visited, 0, sizeof(visited));
	Q.push(shootingEggIndex);

	while(!Q.empty()) {
		Index eggIndex = Q.front();
		eggIndices[eggCount++] = eggIndex;
		Q.pop();

		int rowIndex = eggIndex.rowIndex;
		int colIndex = eggIndex.colIndex;
		int steps[6][2];
		if (rowIndex % 2 == 0) {
			std::memcpy(steps, stepsForEvenRowIndex, sizeof(stepsForEvenRowIndex));
		}
		else {
			std::memcpy(steps, stepsForOddRowIndex, sizeof(stepsForOddRowIndex));
		}
		for (int i = 0; i < 6; i++) {
			int newRowIndex = rowIndex + steps[i][0];
			if (newRowIndex < 0) newRowIndex += NUM_ROWS;
			else if (newRowIndex >= NUM_ROWS) newRowIndex -= NUM_ROWS;
			int newColIndex = colIndex + steps[i][1];

			int numOfCols = (newRowIndex % 2 == 0 ? NUM_COLS-1 : NUM_COLS);
			if (newColIndex >= 0
				&& newColIndex < numOfCols
				&& eggBatchState[newRowIndex][newColIndex] == true
				&& visited[newRowIndex][newColIndex] == false
				&& eggBatchBitmapID[newRowIndex][newColIndex] == shootingEggBitmapID
				&& eggBatchY[newRowIndex] + 0.5 * EGG_HEIGHT > 0) {
				Q.push({newRowIndex, newColIndex});
				visited[newRowIndex][newColIndex] = true;
			}
		}
	}
	for (int i = 0; i < eggCount; i++) {
		Index eggIndex = eggIndices[i];
		eggBatchState[eggIndex.rowIndex][eggIndex.colIndex] = false;
	}
	if (eggCount >= 3) {
		for (int i = 0; i < eggCount; i++) {
			Index eggIndex = eggIndices[i];
			eggBatchState[eggIndex.rowIndex][eggIndex.colIndex] = false;
		}
	}

}

void Screen1View::updateShootingEggAfterCollision() {
	shootingEgg.setBitmap(touchgfx::Bitmap(nextShootingEggBitmapID));
	shootingEggState = IDLE;
	shootingEggX = 107;
	shootingEggY = 280;
}

void Screen1View::updateNextShootingEggAfterCollision() {
	nextShootingEggBitmapID = generateRandomEggBitmapID();
	nextShootingEgg.setBitmap(nextShootingEggBitmapID);
}
uint32_t Screen1View::lcd_rand() {
	seed = (1103515245 * seed + 12345) & 0x7fffffff;
	return seed;
}
uint16_t Screen1View::generateRandomEggBitmapID() {
	uint32_t randomEggBitmapID = lcd_rand() % eggBitmapIDRange;
	return eggBitmapID[randomEggBitmapID];
}


IndexQueue::IndexQueue() {
	head = 0;
	tail = 0;
}

Index IndexQueue::front() {
	return q[head];
}

bool IndexQueue::pop() {
	if (empty()) return false;
	else {
		head = (head+1) % MAX_LEN;
		return true;
	}
}

bool IndexQueue::push(Index index) {
	if (full()) return false;
	else {
		tail = (tail+1) % MAX_LEN;
		q[tail] = index;
		return true;
	}
}
bool IndexQueue::empty() {
	return head == tail;
}
bool IndexQueue::full() {
	return ((tail+1) % MAX_LEN) == head;
}
