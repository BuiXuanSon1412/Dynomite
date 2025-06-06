#ifndef SCREEN1VIEW_HPP
#define SCREEN1VIEW_HPP

#include <gui_generated/screen1_screen/Screen1ViewBase.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>

#define NUM_ROWS 9
#define NUM_COLS 9

class Screen1View : public Screen1ViewBase
{
public:
    Screen1View();
    virtual ~Screen1View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    //void onPlayButtonClicked();

protected:
    touchgfx::Image eggs[NUM_ROWS][NUM_COLS];
private:
    void renderEggImages();
};

#endif // SCREEN1VIEW_HPP
