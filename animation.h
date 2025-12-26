#include"timer.h"

class Animation
{
    timer timer;
    int frameCount;

    public:
        Animation():timer(0),frameCount(0){}
        Animation(int framecount,float length):frameCount(framecount),timer(length){}

        float getLength()const { return timer.getlength();}
        int currentFrame()const 
        {
            return static_cast<int>(timer.getTimer()/timer.getlength()*frameCount);
        }

        void step(float deltat)
        {
            
            timer.step(deltat);
        }



};