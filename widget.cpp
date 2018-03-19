#include "widget.h"
Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    imgLab = new ImgLabel();
    filepath ="";
    resize(1000,800);
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
    if(NULL != imgLab && filepath!="")
    {
        imgLab->setPickup(true);
    }
}
void Widget::slot_act_openfile()
{
    QString p_filepath = QFileDialog::getOpenFileName(this,QString::fromLocal8Bit("选择所要打开的图片"),"","JPG Files (*.jpg)");
    filepath = p_filepath;
    if(p_filepath == "")
        return;
    imgLab->setFilePath(p_filepath);
    imgLab->iniDataset();
    imgLab->showImg();
}
void Widget::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        if(NULL != imgLab)
        {
            imgLab->savePic();
            imgLab->setPickup(false);
        }
    }
}
void Widget::resizeEvent(QResizeEvent *event)
{
    imgLab->resize(event->size());
    if(filepath!="")
        imgLab->showImg();
    QWidget::resizeEvent(event);
}
