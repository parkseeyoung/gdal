#include "widget.h"
Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    imgLab = new ImgLabel("C:\\Users\\CHEN-LIU\\Desktop\\interview\\Fields_Camomiles_Sky_492698.jpg");
    imgLab->resize(1000,800);
    imgLab->iniDataset();
    imgLab->showImg();

    iniMenu();
    iniLayout();
}

Widget::~Widget()
{
}
void Widget::iniMenu()
{
    //加菜单
    m_MenuBar = new QMenuBar(this);

//文件栏
    QMenu * menu_file = new QMenu(QString::fromLocal8Bit("文件"));
    m_MenuBar->addMenu(menu_file);
    //打开文件
    QAction *act_openfile = new QAction(QString::fromLocal8Bit("打开文件"),this);
    connect(act_openfile,SIGNAL(triggered(bool)),this,SLOT(slot_act_openfile()));
    menu_file->addAction(act_openfile);

//工具
    QMenu * menu_tool = new QMenu(QString::fromLocal8Bit("工具"));
    m_MenuBar->addMenu(menu_tool);
    //抠图
    QAction *act_pickupPic = new QAction(QString::fromLocal8Bit("抠图"),this);
    connect(act_pickupPic,SIGNAL(triggered(bool)),this,SLOT(slot_act_pickupPic()));
    menu_tool->addAction(act_pickupPic);
}
void Widget::iniLayout()
{
    QGridLayout *sLayout = new QGridLayout;
    sLayout->setMargin(0);
    sLayout->setMenuBar(m_MenuBar);
    sLayout->addWidget(imgLab,0,0);
    this->setLayout(sLayout);
}
void Widget::slot_act_pickupPic()
{

}
void Widget::slot_act_openfile()
{

}
