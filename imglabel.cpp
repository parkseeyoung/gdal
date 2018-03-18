#include "imglabel.h"
#include <QMessageBox>
#include <QWheelEvent>
#include <QDebug>
ImgLabel::ImgLabel(QString _filePath, QWidget *parent)
    :m_filePath(_filePath),QLabel(parent)
{
    setMouseTracking(false);
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
    //qDebug()<<this->width()<<scale<<dataHeight<<dataWidth;

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
    if(m_scale==1)
        return;
    QPointF curPoint = m_curPos;


    //之前点的实际像素
    qreal ltX = (qreal)curPoint.x()/this->width()*(point_rightBottom.x()-point_leftTop.x())+point_leftTop.x();
    qreal ltY = (qreal)curPoint.y()/this->height()*(point_rightBottom.y()-point_leftTop.y())+point_leftTop.y();

    //思路一
    //通过移动的距离计算移动的像素
    //先计算curPos点的像素，它等于现在的pos的像素，从而计算出左上和右下
//    qreal pltX = ltX - pix_width/m_scale*(event->pos().x()/(qreal)(this->width()))/m_scale;
//    qreal pltY = ltY - pix_height/m_scale*(event->pos().y()/(qreal)(this->height()))/m_scale;


//    qreal prbX = pltX + pix_width/m_scale;
//    qreal prbY = pltY + pix_height/m_scale;

//    //
//    pltX = point_leftTop.x()-(event->pos().x()-m_curPos.x())/(qreal)width()*pix_width/m_scale;
//    pltY = point_leftTop.y()-(event->pos().y()-m_curPos.y())/(qreal)height()*pix_height/m_scale;

//    prbX = point_rightBottom.x()-(event->pos().x()-m_curPos.x())/(qreal)width()*pix_width/m_scale;
//    prbY = point_rightBottom.y()-(event->pos().y()-m_curPos.y())/(qreal)height()*pix_height/m_scale;
//    //
//    point_leftTop.setX(pltX);
//    point_leftTop.setY(pltY);

//    point_rightBottom.setX(prbX);
//    point_rightBottom.setY(prbY);

//    qDebug()<<"m_scale is "<<m_scale;

    //思路二
    //始终保持鼠标所对应的像素不变
    point_leftTop.setX(ltX-(qreal)event->pos().x()/(qreal)this->width()*pix_width/m_scale);
    point_leftTop.setY(ltY-(qreal)event->pos().y()/(qreal)this->height()*pix_height/m_scale);

    point_rightBottom.setX(ltX-(qreal)event->pos().x()/(qreal)this->width()*pix_width/m_scale+pix_width/m_scale);
    point_rightBottom.setY(ltY-(qreal)event->pos().y()/(qreal)this->height()*pix_height/m_scale+pix_height/m_scale);

    //qDebug()<<QString::fromLocal8Bit("该点的像素:")<<(qreal)event->pos().x()/this->width()*(point_rightBottom.x()-point_leftTop.x())+point_leftTop.x();
    m_curPos = event->pos();
    showImg();
}

void ImgLabel::wheelEvent(QWheelEvent *event)
{
    if(event->delta()>0)
    {

        m_scale +=0.2;
        calPix(event->pos());
        qDebug()<<QString::fromLocal8Bit("向上滚了")<<m_scale;
    }
    else if(event->delta() < 0)
    {

        m_scale -=0.2;
        calPix(event->pos());
        qDebug()<<QString::fromLocal8Bit("向下滚了")<<m_scale;
    }
    showImg();
}

/*通过滚轮操作改变当前的比例，根据比例计算截取的大小
 * 根据滚动时的鼠标的位置确定截取的位置
 */
void ImgLabel::calPix(QPoint curPoint)
{
    qDebug()<<QString::fromLocal8Bit("左上坐标为")<<point_leftTop<<QString::fromLocal8Bit("右上坐标为")
           <<point_rightBottom;
    //有个总的width也就是像素，根据比例放大或缩小
    //当前点的实际像素
    qreal ltX = (qreal)curPoint.x()/(qreal)this->width()*(point_rightBottom.x()-point_leftTop.x())+point_leftTop.x();
    qreal ltY = (qreal)curPoint.y()/(qreal)this->height()*(point_rightBottom.y()-point_leftTop.y())+point_leftTop.y();
    //当前点相对于窗体的比例
    qreal rateX = (qreal)curPoint.x()/this->width();
    qreal rateY = (qreal)curPoint.y()/this->height();

    qDebug()<<QString::fromLocal8Bit("当前点相对于窗体的比例")<<rateX<<","<<rateY;

    //计算经过scale后的实际像素宽和高;
    qreal pixW = (qreal)(pix_width)/m_scale;
    qreal pixH = (qreal)(pix_height)/m_scale;

    qDebug()<<QString::fromLocal8Bit("计算经过scale后的实际像素宽和高")<<pixW<<","<<pixH;

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

    qDebug()<<QString::fromLocal8Bit("鼠标点的实际像素宽：")<<ltX
           <<QString::fromLocal8Bit("鼠标点的实际的像素高：")<<ltY;
    point_leftTop.setX(x1);
    point_leftTop.setY(y1);

    point_rightBottom.setX(x2);
    point_rightBottom.setY(y2);
}
