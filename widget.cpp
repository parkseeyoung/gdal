#include "widget.h"
Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    imgLab = new ImgLabel("C:\\Users\\CHEN-LIU\\Desktop\\interview\\Fields_Camomiles_Sky_492698.jpg",this);
    imgLab->resize(1000,800);
    imgLab->iniDataset();
    imgLab->showImg();
}

Widget::~Widget()
{
}
