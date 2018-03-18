#ifndef IMGLABEL_H
#define IMGLABEL_H
#include <QLabel>
#include "gdal_priv.h"
#include <QVector>
class ImgLabel : public QLabel
{
    Q_OBJECT
public:
    ImgLabel(QString _filePath,QWidget *parent=0);
    ~ImgLabel();
private:
    void wheelEvent(QWheelEvent * event);
    //测试toPix函数
    void mousePressEvent(QMouseEvent *event);
    //图像漫游操作
    void mouseMoveEvent(QMouseEvent * event);
    //转化函数
    void calPix(QPoint curPoint);
    //绘图（用于显示抠图的点）
    void paintEvent(QPaintEvent *event);

private:
    //文件路径
    QString m_filePath;
    //图片的总像素大小
    qreal pix_width;
    qreal pix_height;
    //label左上和右下角所对应的图片的像素
    QPointF point_leftTop;
    QPointF point_rightBottom;

    //Dataset
    GDALDataset *m_pDataset;
    //当前缩放的比例
    qreal m_scale;

    //当前鼠标点击的位置
    QPointF m_curPos;

    //开始抠图
    bool m_pick_up;

    //定义一个容器，放刺出的点的坐标（最后统一计算像素）
    QVector<QPoint>vec_point;
public:
    //初始化dataset
    void iniDataset();

    void showImg();

    //设置抠图状态
    void setPickup(bool status);
};

#endif // IMGLABEL_H
