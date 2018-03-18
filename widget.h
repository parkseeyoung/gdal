#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtWidgets>
#include "imglabel.h"
class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = 0);
    ~Widget();
private:
    ImgLabel * imgLab;
    //菜单
    QMenuBar * m_MenuBar;

    //文件路径
    QString filepath;

private:
    //初始化菜单
    void iniMenu();
    //布局
    void iniLayout();

private slots:
    void slot_act_openfile();
    void slot_act_pickupPic();

};

#endif // WIDGET_H
