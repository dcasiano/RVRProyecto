#include "Serializable.h"

#include <string.h>

class Entity: public Serializable
{
public:
    Entity(const char * _n, int16_t _x, int16_t _y):x(_x),y(_y), isAlive(true)
    {
        strncpy(name, _n, 80);
    };

    virtual ~Entity(){};

    void to_bin()
    {
        size_t size=2*sizeof(int16_t)+80+sizeof(bool);
        alloc_data(size);
        char *tmp = _data;
        memcpy(tmp,name,80);
        tmp+=80;
        memcpy(tmp,&x,sizeof(int16_t));
        tmp+=sizeof(int16_t);
        memcpy(tmp,&y,sizeof(int16_t));
        tmp+=sizeof(int16_t);
        memcpy(tmp,&isAlive,sizeof(bool));
    }

    int from_bin(char * data)
    {
        char *tmp = data;
        memcpy(name,tmp,80);
        tmp+=80;
        memcpy(&x,tmp,sizeof(int16_t));
        tmp+=sizeof(int16_t);
        memcpy(&y,tmp,sizeof(int16_t));
        tmp+=sizeof(int16_t);
        memcpy(&isAlive,tmp,sizeof(bool));
        
        return 0;
    }


public:
    char name[80];

    int16_t x;
    int16_t y;

    enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };
    Direction dir;

    bool isAlive;
};