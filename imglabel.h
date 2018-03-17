#ifndef IMGLABEL_H
#define IMGLABEL_H
#include <QLabel>
#include "gdal_priv.h"

class ImgLabel : public QLabel
{
    Q_OBJECT
public:
    ImgLabel(QString _filePath,QWidget *parent);
    ~ImgLabel();
private:
    void wheelEvent(QWheelEvent * event);
    //测试toPix函数
    void mousePressEvent(QMouseEvent *event);
    //图像漫游操作
    void mouseMoveEvent(QMouseEvent * event);
    //转化函数
    void calPix(QPoint curPoint);

private:
    //文件路径
    QString m_filePath;
    //图片的总像素大小
    int pix_width;
    int pix_height;
    //label左上和右下角所对应的图片的像素
    QPoint point_leftTop;
    QPoint point_rightBottom;

    //Dataset
    GDALDataset *m_pDataset;
    //当前缩放的比例
    qreal m_scale;

    //当前鼠标点击的位置
    QPointF m_curPos;
public:
    //初始化dataset
    void iniDataset();

    void showImg();
};

#endif // IMGLABEL_H
