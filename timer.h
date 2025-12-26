


class timer
{
    float length,time;
    bool timeout;
    public:
        timer(float length):length(length),time(0),timeout(false)
        {

        }

        void step(float deltat)
        {
            time+=deltat;
            if(time>=length)
            {
                time-=length;
                timeout=true;
            }
        }

        bool isTimeout() const {return timeout;}
        float getTimer() const {return time;}
        float getlength() const{return length;}
        void reset(){time=0, timeout=false;}

};