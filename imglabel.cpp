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
    //通过移动的距离计算移动的像素
    qreal changeX = event->pos().x() - m_curPos.x();
    qreal changeY = event->pos().y() - m_curPos.y();

    qDebug()<<changeX<<changeY;

    //当前像素宽和高
    int p_pixWidth = point_rightBottom.x()-point_leftTop.x();
    int p_pixHeight = point_rightBottom.y()-point_leftTop.y();

    qreal changePixX = changeX * p_pixWidth/this->width()/m_scale/m_scale;
    qreal changePixY = (qreal)changeY * p_pixHeight/this->height()/m_scale/m_scale;

    qDebug()<<p_pixWidth<<p_pixHeight<<changePixX<<changePixY;
    int ltX = point_leftTop.x() - changePixX;
    int ltY = point_leftTop.y() - changePixY;
    int rbX = point_rightBottom.x() - changePixX;
    int rbY = point_rightBottom.y() - changePixY;
    if(ltX<=0||ltY<=0||rbX>=pix_width||rbY>=pix_height)
        return;
    point_leftTop = QPoint(ltX,ltY);
    point_rightBottom = QPoint(rbX,rbY);
    showImg();
}

void ImgLabel::wheelEvent(QWheelEvent *event)
{
    if(event->delta()>0)
    {

        m_scale +=0.1;
        calPix(event->pos());
        qDebug()<<QString::fromLocal8Bit("向上滚了")<<m_scale;
    }
    else if(event->delta() < 0)
    {

        m_scale -=0.1;
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
    int ltX = (qreal)curPoint.x()/this->width()*(point_rightBottom.x()-point_leftTop.x())+point_leftTop.x();
    int ltY = (qreal)curPoint.y()/this->height()*(point_rightBottom.y()-point_leftTop.y())+point_leftTop.y();
    //当前点相对于窗体的比例
    qreal rateX = (qreal)curPoint.x()/this->width();
    qreal rateY = (qreal)curPoint.y()/this->height();

    qDebug()<<QString::fromLocal8Bit("当前点相对于窗体的比例")<<rateX<<","<<rateY;

    //计算经过scale后的实际像素宽和高;
    qreal pixW = (qreal)(pix_width)/m_scale;
    qreal pixH = (qreal)(pix_height)/m_scale;

    qDebug()<<QString::fromLocal8Bit("计算经过scale后的实际像素宽和高")<<pixW<<","<<pixH;

    int x1 = ltX-rateX*pixW;
    int y1 = ltY-rateY*pixH;
    int x2 = x1+pixW;
    int y2 = y1+pixH;

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
