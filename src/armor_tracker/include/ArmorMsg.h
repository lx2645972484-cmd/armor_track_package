#ifndef ARMOR_MSG_H
#define ARMOR_MSG_H

class Point3D
{

public:
    Point3D(){}
    double x;
    double y;
    double z;
};

class ArmorMsg
{

public:
    ArmorMsg()
    {
    }   
    double yaw;
};

#endif