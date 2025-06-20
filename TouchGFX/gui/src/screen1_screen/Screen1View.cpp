#include <gui/screen1_screen/Screen1View.hpp>

#include <cmsis_os.h>
#include <cmath>
#include <cstdio>
#include <utility>
#include <cstring>
#include <queue>

extern uint16_t controllerX, controllerY;
extern uint16_t prevControllerX, prevControllerY;
extern uint16_t currentScore;
extern uint16_t highScore;

Screen1View::Screen1View()
{
	// prepare for game flow
	gameState = false;

	// from the start, there will be 3 types of eggs
	eggBitmapIDRange = 3;

	// generate seed for random generator
	seed = 1;
}

void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();

    remove(scoreContainer);

    add(shootingEgg);
    shootingEgg.setVisible(false);
    shootingEgg.invalidate();

    add(nextShootingEgg);
    nextShootingEgg.setVisible(false);
    nextShootingEgg.invalidate();
    for (int i = 0; i < NUM_ROWS; i++) {
        for (int j = 0; j < NUM_COLS; j++) {
      		add(eggBatch[i][j]);
       	}
    }

    shootingLine.setVisible(false);
    shootingLine.invalidate();

    realtimeScoreTextArea.setVisible(false);
    realtimeScoreTextArea.invalidate();
    add(scoreContainer);
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}

void Screen1View::onPlayButtonClicked() {

	scoreContainer.setVisible(false);
	scoreContainer.invalidate();

	renderRealtimeScoreTextArea();

	gameState = true;
	startRowIndex = 4;
	currentScore = 0;
	dEggBatchY = 0.03;


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
			updateHighScore();
			renderScoreContainer();
			return;
		}
		if (eggBatchY[startRowIndex] + 0.5 * EGG_HEIGHT > LIMIT_Y) {
			gameState = false;
			return;
		}
		updateEggBatch();
		updateShootingEgg();
		updateShootingLine();

		if (shootingEggState == AIRBORNE) {
			Index shootingEggIndex = detectCollisionBetweenShootingEggAndEggBatch();
			// detect which eggs shooting egg can drop and update 'eggBatchState'
			if (shootingEggIndex.rowIndex != -1 && shootingEggIndex.colIndex != -1) {
				uint16_t additionalScore = updateEggBatchAfterCollision(shootingEggIndex);
				updateCurrentScore(additionalScore);
				updateShootingEggAfterCollision();
				updateNextShootingEggAfterCollision();
			}
		}

		renderEggBatch();
		renderShootingEgg();
		renderNextShootingEgg();
		renderShootingLine();
		renderRealtimeScoreTextArea();
		invalidate();
		lastUpdateTickCount = osKernelGetTickCount();
	}

}

void Screen1View::initializeEggBatch() {
	int i = startRowIndex;
	float lastY = BASE_Y;
	do {
		// Y coordinate
		if (lastY == BASE_Y) eggBatchY[i] = startRowIndex * EGG_HEIGHT + 0.5 * EGG_HEIGHT;
		else eggBatchY[i] = lastY - EGG_HEIGHT;
		lastY = eggBatchY[i];

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
			eggBatch[i][j].moveTo(std::round(padding + EGG_WIDTH * j), std::round(eggBatchY[i] - 0.5 * EGG_HEIGHT));
			eggBatchState[i][j] = true;
			eggBatch[i][j].setVisible(eggBatchState[i][j]);
		}
		i = i-1;
		if (i < 0) i += NUM_ROWS;
	} while (i != startRowIndex);
}

void Screen1View::renderEggBatch() {
	for (int i = 0; i < NUM_ROWS; i++) {
		int numOfCols = (i%2 == 0 ? NUM_COLS-1 : NUM_COLS);
		for (int j = 0; j < numOfCols; j++) {
			eggBatch[i][j].setBitmap(touchgfx::Bitmap(eggBatchBitmapID[i][j]));
			eggBatch[i][j].moveTo(eggBatch[i][j].getX(), std::round(eggBatchY[i] - 0.5 * EGG_HEIGHT));
			eggBatch[i][j].setVisible(eggBatchState[i][j]);
			//eggBatch[i][j].invalidate();
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
	shootingEggX = SCREEN_WIDTH * 0.5;
	shootingEggY = BASE_Y;
	dShootingEgg = 2;
	shootingEggState = IDLE;

	shootingEggBitmapID = generateRandomEggBitmapID();
	shootingEgg.setBitmap(touchgfx::Bitmap(shootingEggBitmapID));
	shootingEgg.setXY(std::round(shootingEggX - 0.5 * EGG_WIDTH), (int)std::round(BASE_Y - 0.5 * EGG_HEIGHT));
	shootingEgg.setVisible(true);
}
void Screen1View::updateShootingEgg() {
	//float dx, dy;
	float dx = controllerX - 127.5f;
	float dy = controllerY - 127.5f;
	float d = sqrt(dx * dx + dy * dy);

	switch (shootingEggState) {
	case IDLE:
		if (dy > 5) {
			shootingEggState = READY;
		}
		else {
			dShootingEggX = 0;
			dShootingEggY = 0;
		}
		break;
	case READY:
		if (abs(dx) <= 5 && abs(dy) <= 5) {
			dx = prevControllerX - 127.5f;
			dy = prevControllerY - 127.5f;

			if (abs(dy / dx) < (BASE_Y - LIMIT_Y) / (0.5 * SCREEN_WIDTH)) {
				dx = (dx < 0 ? -0.5 * SCREEN_WIDTH : 0.5 * SCREEN_WIDTH);
				dy = BASE_Y - LIMIT_Y;
			}

			d = sqrt(dx * dx + dy * dy);

			dShootingEggX = -dx / d;
			dShootingEggY = -dy / d;

			shootingLineEndX = (int) (0.5 * SCREEN_WIDTH + dShootingEggX * 150);
			shootingLineEndY = (int) (BASE_Y + dShootingEggY * 150);

			shootingEggState = AIRBORNE;
		}
		else if (dy > 0) {
			if (abs(dy / dx) < (BASE_Y - LIMIT_Y) / (0.5 * SCREEN_WIDTH)) {
				dx = (dx < 0 ? -0.5 * SCREEN_WIDTH : 0.5 * SCREEN_WIDTH);
				dy = BASE_Y - LIMIT_Y;
				d = sqrt(dx * dx + dy * dy);
			}
			dShootingEggX = -dx / d;
			dShootingEggY = -dy / d;

			shootingLineEndX = (int) (0.5 * SCREEN_WIDTH + dShootingEggX * 150);
			shootingLineEndY = (int) (BASE_Y + dShootingEggY * 150);
		}
		break;
	case AIRBORNE:
		if (shootingEggX + 0.5 * EGG_WIDTH >= SCREEN_WIDTH
			|| shootingEggX - 0.5 * EGG_WIDTH <= 0) {
			dShootingEggX = -dShootingEggX;
		}
		shootingEggX += dShootingEggX * dShootingEgg;
		shootingEggY += dShootingEggY * dShootingEgg;
		break;
	default:
		break;
	}

}

void Screen1View::renderShootingEgg() {
	shootingEgg.setBitmap(touchgfx::Bitmap(shootingEggBitmapID));
	shootingEgg.moveTo((int)std::round(shootingEggX - 0.5 * EGG_WIDTH), std::round(shootingEggY - 0.5 * EGG_HEIGHT));
	//shootingEgg.invalidate();
}

void Screen1View::initializeNextShootingEgg() {
	nextShootingEggBitmapID = generateRandomEggBitmapID();
	nextShootingEgg.setXY(10, BASE_Y - 0.5 * EGG_HEIGHT);
	nextShootingEgg.setVisible(true);
}
void Screen1View::updateNextShootingEgg() {
	//sampleEgg.moveTo(sampleEgg.getX(), sampleEgg.getY() + 2);
}

void Screen1View::renderNextShootingEgg() {
	nextShootingEgg.setBitmap(touchgfx::Bitmap(nextShootingEggBitmapID));
	//nextShootingEgg.invalidate();
}

void Screen1View::initializeShootingLine() {
	shootingLine.setStart((int)(0.5 * SCREEN_WIDTH), BASE_Y);
	shootingLine.setVisible(false);
	shootingLine.invalidate();
}

void Screen1View::updateShootingLine() {
	if (shootingEggState == READY
		|| shootingEggState == AIRBORNE) {
		shootingLine.setEnd(shootingLineEndX, shootingLineEndY);
	}
}

void Screen1View::renderShootingLine() {
	if (shootingEggState == READY || shootingEggState == AIRBORNE) {
		shootingLine.setVisible(true);
		shootingLine.invalidate();
	}
	else {
		shootingLine.setVisible(false);
		shootingLine.invalidate();
	}
}

void Screen1View::updateCurrentScore(uint16_t additionalScore) {
	currentScore = currentScore + additionalScore;
}

void Screen1View::updateHighScore() {
	highScore = (currentScore > highScore ? currentScore : highScore);
}

void Screen1View::renderRealtimeScoreTextArea() {
	Unicode::snprintf(realtimeScoreTextAreaBuffer, sizeof(realtimeScoreTextAreaBuffer), "%05d", (int)(currentScore));
	realtimeScoreTextArea.setVisible(true);
	realtimeScoreTextArea.invalidate();
}
void Screen1View::renderScoreContainer() {
	Unicode::snprintf(currentScoreTextAreaBuffer, sizeof(currentScoreTextAreaBuffer), "%05d", (int)(currentScore));
	Unicode::snprintf(ScoreTextAreaBuffer, sizeof(highScoreTextAreaBuffer), "%05d", (int)(highScore));

	scoreContainer.setVisible(true);
	scoreContainer.invalidate();
}
Index Screen1View::detectCollisionBetweenShootingEggAndEggBatch() {
	if (shootingEggY - 0.5 * EGG_HEIGHT > eggBatchY[startRowIndex] + 0.5 * EGG_HEIGHT) return {-1, -1};

	int i = startRowIndex;
	do {
		int numOfCols = (i % 2 == 0 ? NUM_COLS-1 : NUM_COLS);
		int newNumOfCols = (numOfCols == NUM_COLS ? NUM_COLS-1 : NUM_COLS);

		for (int j = 0; j < numOfCols; j++) {
			if (eggBatchState[i][j]) {
				float x = eggBatch[i][j].getX() + 0.5 * EGG_WIDTH;
				float y = eggBatchY[i];
				if (dShootingEggX <= 0) {
					if (shootingEggX - 0.5 * EGG_WIDTH > x - 0.5 * EGG_WIDTH
						&& shootingEggX - 0.5 * EGG_WIDTH < x + 0.5 * EGG_WIDTH) {
						if (shootingEggY - 0.5 * EGG_HEIGHT > y
							&& shootingEggY - 0.5 * EGG_HEIGHT < y + 0.5 * EGG_WIDTH) {
							// bottom-right
							int rowIndex = (i+1) % NUM_ROWS;
							int colIndex = (i % 2 == 0 ? j+1 : j);
							if (colIndex < newNumOfCols
									&& (eggBatchState[rowIndex][colIndex] == false
									|| i == startRowIndex)
								) return {rowIndex, colIndex};
						}
						else if (shootingEggY - 0.5 * EGG_HEIGHT > y - 0.5 * EGG_WIDTH
								&& shootingEggY - 0.5 * EGG_HEIGHT < y) {
							// right
							int rowIndex = i;
							int colIndex = j+1;
							if (colIndex < numOfCols
								&& eggBatchState[rowIndex][colIndex] == false) return {rowIndex, colIndex};
						}
						else if (shootingEggY - 0.5 * EGG_HEIGHT < y - 0.5 * EGG_WIDTH
								&& shootingEggY - 0.5 * EGG_HEIGHT > y - 1.5 * EGG_WIDTH) {
								// top-right
								int rowIndex = (i-1 < 0 ? i-1+NUM_ROWS : i-1);
								int colIndex = (i % 2 == 0 ? j+1 : j);
								if (colIndex < newNumOfCols
									&& eggBatchState[rowIndex][colIndex] == false) return {rowIndex, colIndex};

						}
					}
					else if (shootingEggX + 0.5 * EGG_WIDTH > x - 0.5 * EGG_WIDTH
							&& shootingEggX + 0.5 * EGG_WIDTH < x + 0.5 * EGG_WIDTH) {
						if (shootingEggY - 0.5 * EGG_HEIGHT < y + 0.5 * EGG_HEIGHT) {
							// bottom-left
							int rowIndex = (i+1) % NUM_ROWS;
							int colIndex = (i % 2 == 0 ? j : j-1);
							if (colIndex >= 0
								&& (eggBatchState[rowIndex][colIndex] == false
									|| i == startRowIndex)
								) return {rowIndex, colIndex};
						}
					}
				}
				else if (dShootingEggX > 0) {
					if (shootingEggX + 0.5 * EGG_WIDTH > x - 0.5 * EGG_WIDTH
						&& shootingEggX + 0.5 * EGG_WIDTH < x + 0.5 * EGG_WIDTH) {
						if (shootingEggY - 0.5 * EGG_HEIGHT > y
							&& shootingEggY - 0.5 * EGG_HEIGHT < y + 0.5 * EGG_WIDTH) {
							// bottom-left
							int rowIndex = (i+1) % NUM_ROWS;
							int colIndex = (i % 2 == 0 ? j : j-1);
							if (colIndex >= 0
								&& (eggBatchState[rowIndex][colIndex] == false
									|| i == startRowIndex)
								) return {rowIndex, colIndex};
						}
						else if (shootingEggY - 0.5 * EGG_HEIGHT > y - 0.5 * EGG_WIDTH
								&& shootingEggY - 0.5 * EGG_HEIGHT < y) {
							// left
							int rowIndex = i;
							int colIndex = j-1;
							if (colIndex >= 0
								&& eggBatchState[rowIndex][colIndex] == false) return {rowIndex, colIndex};
						}
						else if (shootingEggY - 0.5 * EGG_HEIGHT > y - 1.5 * EGG_WIDTH
								&& shootingEggY - 0.5 * EGG_HEIGHT < y - 0.5 * EGG_WIDTH) {
								// top-left
								int rowIndex = (i-1 < 0 ? i-1+NUM_ROWS : i-1);
								int colIndex = (i % 2 == 0 ? j : j-1);
								if (colIndex >= 0
									&& eggBatchState[rowIndex][colIndex] == false) return {rowIndex, colIndex};
							}

					}
					else if (shootingEggX - 0.5 * EGG_WIDTH < x + 0.5 * EGG_WIDTH
							&& shootingEggX - 0.5 * EGG_WIDTH > x - 0.5 * EGG_WIDTH) {
						if (shootingEggY - 0.5 * EGG_HEIGHT < y + 0.5 * EGG_HEIGHT) {
							// bottom-right
							int rowIndex = (i+1) % NUM_ROWS;
							int colIndex = (i % 2 == 0 ? j+1 : j);
							if (colIndex < newNumOfCols
								&& (eggBatchState[rowIndex][colIndex] == false
									|| i == startRowIndex)
								) return {rowIndex, colIndex};
						}
					}
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

uint16_t Screen1View::updateEggBatchAfterCollision(Index shootingEggIndex) {
	// temporary update shootingEgg on eggBatch
	int rowIndex = shootingEggIndex.rowIndex;
	int colIndex = shootingEggIndex.colIndex;

	eggBatchBitmapID[rowIndex][colIndex] = shootingEggBitmapID;
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

	Index eggIndices[MAX_LEN];
	uint16_t eggCount = 0;

	bool visited[NUM_ROWS][NUM_COLS];
	memset(visited, 0, sizeof(visited));
	visited[rowIndex][colIndex] = true;
	IndexQueue Q;
	Q.push(shootingEggIndex);

	int steps[6][2];

	while(!Q.empty()) {
		Index eggIndex = Q.front();
		Q.pop();
		eggIndices[eggCount++] = eggIndex;

		int currentRowIndex = eggIndex.rowIndex;
		int currentColIndex = eggIndex.colIndex;

		if (currentRowIndex % 2 == 0) {
			std::memcpy(steps, stepsForEvenRowIndex, sizeof(stepsForEvenRowIndex));
		}
		else {
			std::memcpy(steps, stepsForOddRowIndex, sizeof(stepsForOddRowIndex));
		}
		for (int i = 0; i < 6; i++) {
			int newRowIndex = currentRowIndex + steps[i][0];
			if (newRowIndex < 0) newRowIndex += NUM_ROWS;
			else if (newRowIndex >= NUM_ROWS) newRowIndex -= NUM_ROWS;
			int newColIndex = currentColIndex + steps[i][1];

			int numOfCols = (newRowIndex % 2 == 0 ? NUM_COLS-1 : NUM_COLS);
			if (newColIndex >= 0
				&& newColIndex < numOfCols
				&& eggBatchY[newRowIndex] + 0.5 * EGG_HEIGHT > 0
				&& eggBatchState[newRowIndex][newColIndex] == true
				&& visited[newRowIndex][newColIndex] == false
				&& eggBatchBitmapID[newRowIndex][newColIndex] == shootingEggBitmapID) {
				Q.push({newRowIndex, newColIndex});
				visited[newRowIndex][newColIndex] = true;
			}
		}
	}

	if (eggCount >= 3) {
		for (int i = 0; i < eggCount; i++) {
			eggBatchState[eggIndices[i].rowIndex][eggIndices[i].colIndex] = false;
		}

		// detect new startRowIndex ~ i
		int i = startRowIndex;
		do {
			int numOfCols = (i % 2 == 0 ? NUM_COLS-1 : NUM_COLS);
			bool empty = true;
			for (int j = 0; j < numOfCols; j++) {
				if (eggBatchState[i][j]) {
					empty = false;
					break;
				}
			}
			if (!empty) break;
			i = i-1;
			if (i < 0) i = i + NUM_ROWS;
		} while (i != startRowIndex);

		// update y coordinate of new non-displayed rows
		if (i != startRowIndex) {
			float lastY = eggBatchY[(startRowIndex + 1) % NUM_ROWS];
			int j = startRowIndex;
			do {
				eggBatchY[j] = lastY - EGG_HEIGHT;
				lastY = eggBatchY[j];

				int numOfCols = (j % 2 ? NUM_COLS-1 : NUM_COLS);
				for (int k = 0; k < numOfCols; k++) {
					eggBatchState[j][k] = true;
				}

				j = j-1;
				if (j < 0) j = j + NUM_ROWS;
			} while (j != i);

			// update startRowIndex
			startRowIndex = i;
		}
	}

	return (eggCount < 3 ? 0 : eggCount);
}

void Screen1View::updateShootingEggAfterCollision() {
	shootingEggBitmapID = nextShootingEggBitmapID;
	shootingEggState = IDLE;
	shootingEggX = SCREEN_WIDTH * 0.5;
	shootingEggY = BASE_Y;
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
		q[tail] = index;
		tail = (tail+1) % MAX_LEN;
		return true;
	}
}
bool IndexQueue::empty() {
	return head == tail;
}
bool IndexQueue::full() {
	return ((tail+1) % MAX_LEN) == head;
}
