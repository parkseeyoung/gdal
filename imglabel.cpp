#include "imglabel.h"
#include <QMessageBox>
#include <QWheelEvent>
#include <QPainter>
#include <QDebug>
ImgLabel::ImgLabel(QString _filePath, QWidget *parent)
    :m_filePath(_filePath),QLabel(parent)
{
    setMouseTracking(false);
    m_pick_up = false;
    m_scale = 1;
}
ImgLabel::~ImgLabel()
{
    if(m_pDataset != NULL)
    {
        GDALClose((GDALDatasetH) m_pDataset);
        m_pDataset = NULL;
    }
}
void ImgLabel::iniDataset()
{
    GDALAllRegister();
    m_pDataset = (GDALDataset*)GDALOpen(qPrintable(m_filePath), GA_ReadOnly);
    if (m_pDataset == NULL)
    {
        QMessageBox::information(this,QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("指定的文件不能打开！"));
        return;
    }
    //设置投影坐标范围
    pix_width = m_pDataset->GetRasterXSize();
    pix_height = m_pDataset->GetRasterYSize();

    point_leftTop = QPoint(0,0);
    point_rightBottom = QPoint(pix_width,pix_height);

    double padfTransform[6] = {0.0};
    m_pDataset->GetGeoTransform(padfTransform);
}

void ImgLabel::showImg()
{
    int dataWidth = point_rightBottom.x()-point_leftTop.x();
    int dataHeight = point_rightBottom.y()-point_leftTop.y();
    int dataBands = m_pDataset->GetRasterCount();

    int band_list[3] = {1,2,3};
    qreal p_width = this->width();
    qreal scale = p_width / (dataHeight > dataWidth ? dataHeight : dataWidth);

    int iSize = GDALGetDataTypeSize(GDT_Byte) / 8;
    int iScaleWidth = static_cast<int>(dataWidth*scale+0.5);

    int iScaleHeight = static_cast<int>(dataHeight*scale+0.5);

    iScaleWidth = (iScaleWidth*8+31)/32*4;

    unsigned char* pBuffer = new unsigned char[iScaleWidth*iScaleHeight*dataBands];

    CPLErr err = m_pDataset->RasterIO(GF_Read, point_leftTop.x(), point_leftTop.y(), dataWidth, dataHeight, pBuffer, iScaleWidth, iScaleHeight,
        GDT_Byte, dataBands, band_list, iSize*dataBands, iSize*iScaleWidth*dataBands, iSize);	//读取所有波段的一行数据

    unsigned char* pDataBuffer = NULL;
    if (dataBands >=3 )
    {
        pDataBuffer = pBuffer;
    }
    else
    {
        pDataBuffer = new unsigned char[iScaleWidth*iScaleHeight*3];
        for (int i=0; i<iScaleWidth*iScaleHeight*3; i++)
            pDataBuffer[i] = pBuffer[i/3];

        delete []pBuffer;
    }

    QImage QImg(pDataBuffer, iScaleWidth, iScaleHeight, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(QImg);
    delete []pDataBuffer;
    setPixmap(pixmap);
    this->resize(pixmap.size());
}
void ImgLabel::mousePressEvent(QMouseEvent *event)
{
    m_curPos = event->pos();
}
void ImgLabel::mouseMoveEvent(QMouseEvent *event)
{
    if(m_scale==1 || m_pick_up == true)
        return;
    QPointF curPoint = m_curPos;


    //之前点的实际像素
    qreal ltX = (qreal)curPoint.x()/this->width()*(point_rightBottom.x()-point_leftTop.x())+point_leftTop.x();
    qreal ltY = (qreal)curPoint.y()/this->height()*(point_rightBottom.y()-point_leftTop.y())+point_leftTop.y();

    //思路一
    //通过移动的距离计算移动的像素
    //先计算curPos点的像素，它等于现在的pos的像素，从而计算出左上和右下

    //思路二
    //始终保持鼠标所对应的像素不变
    qreal pltX = ltX-(qreal)event->pos().x()/(qreal)this->width()*pix_width/m_scale;
    qreal pltY = ltY-(qreal)event->pos().y()/(qreal)this->height()*pix_height/m_scale;
    qreal prbX = ltX-(qreal)event->pos().x()/(qreal)this->width()*pix_width/m_scale+pix_width/m_scale;
    qreal prbY = ltY-(qreal)event->pos().y()/(qreal)this->height()*pix_height/m_scale+pix_height/m_scale;

    if(pltX<=0||pltY<=0||prbX>=pix_width||prbY>=pix_height)
    {
        m_curPos = event->pos();
        return;
    }
    point_leftTop.setX(pltX);
    point_leftTop.setY(pltY);

    point_rightBottom.setX(prbX);
    point_rightBottom.setY(prbY);

    m_curPos = event->pos();
    showImg();
}

void ImgLabel::wheelEvent(QWheelEvent *event)
{
    if(m_pick_up == true)
        return;
    if(event->delta()>0)
    {

        m_scale +=0.2;
        calPix(event->pos());
    }
    else if(event->delta() < 0)
    {

        m_scale -=0.2;
        calPix(event->pos());
    }
    showImg();
}

/*通过滚轮操作改变当前的比例，根据比例计算截取的大小
 * 根据滚动时的鼠标的位置确定截取的位置
 */
void ImgLabel::calPix(QPoint curPoint)
{
    //有个总的width也就是像素，根据比例放大或缩小
    //当前点的实际像素
    qreal ltX = (qreal)curPoint.x()/(qreal)this->width()*(point_rightBottom.x()-point_leftTop.x())+point_leftTop.x();
    qreal ltY = (qreal)curPoint.y()/(qreal)this->height()*(point_rightBottom.y()-point_leftTop.y())+point_leftTop.y();
    //当前点相对于窗体的比例
    qreal rateX = (qreal)curPoint.x()/this->width();
    qreal rateY = (qreal)curPoint.y()/this->height();


    //计算经过scale后的实际像素宽和高;
    qreal pixW = (qreal)(pix_width)/m_scale;
    qreal pixH = (qreal)(pix_height)/m_scale;


    qreal x1 = ltX-rateX*pixW;
    qreal y1 = ltY-rateY*pixH;
    qreal x2 = x1+pixW;
    qreal y2 = y1+pixH;

    if(x1<=0)
        x1=0;
    if(y1<=0)
        y1=0;
    if(x2>=pix_width)
        x2=pix_width;
    if(y2>=pix_height)
        y2=pix_height;
    if(x1==0&&y1==0&&x2==pix_width&&y2==pix_height)
        m_scale = 1;

    point_leftTop.setX(x1);
    point_leftTop.setY(y1);

    point_rightBottom.setX(x2);
    point_rightBottom.setY(y2);
}

void ImgLabel::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);
    if(m_pick_up == false)
        return;
    //按顺序连接点然后显示
    QPainter p_painter(this);
    QPainterPath p_path;
    for(int i = 0;i<vec_point.count();i++)
    {
        if(i==0)
            p_path.moveTo(vec_point.at(i));
        else
        {
            p_path.lineTo(vec_point.at(i));
        }
    }
    QPen p_pen(Qt::red);
    p_painter.setPen(p_pen);
    p_painter.drawRect(10,10,400,400);
}

void ImgLabel::setPickup(bool status)
{
    m_pick_up =  status;
}
