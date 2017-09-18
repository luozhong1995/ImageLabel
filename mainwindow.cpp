#include "mainwindow.h"
#include <QApplication>
#include <qmainwindow.h>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QDateTime>
#include <QDesktopServices>
#include <QDebug>
#include <QClipboard>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <qfiledialog.h>
#include <qmenu.h>
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qkeysequence.h>
#include <stdlib.h>
#include <algorithm>
#include <math.h>
using namespace std;





#pragma execution_character_set("utf-8") 


// 选中矩形8个拖拽点小矩形的宽高;
#define STRETCH_RECT_WIDTH 6
#define STRETCH_RECT_HEIGHT 6

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	,m_currentCaptureState(InitCapture)
{
	/*
	openAction = new QAction(tr("&Open"), this);
	openAction->setShortcut(QKeySequence::Open);
	openAction->setStatusTip(tr("Open a file."));
	QMenu *file = menuBar()->addMenu(tr("&File"));
	file->addAction(openAction);

	QToolBar *toolBar = addToolBar(tr("&File"));
	toolBar->addAction(openAction);
	connect(openAction, SIGNAL(&QAction::triggered), this, SLOT(MainWindow::loadBackgroundPixmap));
	*/
	initWindow();
	initStretchRect();
	loadBackgroundPixmap();
}

MainWindow::~MainWindow()
{

}

void MainWindow::initWindow()
{
	this->setMouseTracking(true);

	this->setWindowFlags(Qt::FramelessWindowHint);

	setWindowState(Qt::WindowActive | Qt::WindowFullScreen);

	//setWindowState(Qt::WindowActive);
	this->setMouseTracking(true);
}

void MainWindow::initStretchRect()
{
	m_stretchRectState = NotSelect;      //NotSelect=-1 
	m_topLeftRect = QRect(0 , 0 , 0 ,0);//左上角坐标，长度和宽度
	m_topRightRect = QRect(0, 0, 0, 0);
	m_bottomLeftRect = QRect(0, 0, 0, 0);
	m_bottomRightRect = QRect(0, 0, 0, 0);

	m_leftCenterRect = QRect(0, 0, 0, 0);
	m_topCenterRect = QRect(0, 0, 0, 0);
	m_rightCenterRect = QRect(0, 0, 0, 0);
	m_bottomCenterRect = QRect(0, 0, 0, 0);
}

// 获取当前屏幕图片;
void MainWindow::loadBackgroundPixmap()
{

	
	QString path = "C:/screenshot.txt";
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qDebug() << "Can't open the file!" << endl;
	}

	while (!file.atEnd()) {
		QByteArray line = file.readLine();
		QString str(line);
		QStringList sections = str.split(" "); //把每一个块装进一个QStringList中
		shotPath= sections[0];
		txtPath = sections[1];
	}
	picPath = QFileDialog::getOpenFileName(this, tr("Open Image"), ".", tr("Image Files(*.jpg *.png)"));
	
	bool flag=m_loadPixmap.load(picPath);
	//int init_x =m_loadPixmap.width();
	//int init_y =m_loadPixmap.height();
	//m_loadPixmap = m_loadPixmap.scaled(1600, 900, Qt::KeepAspectRatio);
	//double scale_x = init_x*1.0 /1600;
	//double scale_y = init_y*1.0/ 900;
	m_screenwidth = m_loadPixmap.width();
	m_screenheight = m_loadPixmap.height();
	this->resize(m_screenwidth,m_screenheight);
	QFile file1("c:/debug.txt");
	if(!file1.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
	{
		QMessageBox::warning(this, "sdf", "can't open", QMessageBox::Yes);
	}
	QTextStream in(&file1);
	in <<m_screenwidth<<" "<<m_screenheight<<" "<<picPath<<"\n";
	file1.close();
	//m_loadPixmap = QPixmap(picPath,0,Qt::AutoColor);
	//m_loadPixmap = QPixmap::grabWindow(QApplication::desktop()->winId()); //抓取当前屏幕的图片;

}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
	m_stretchRectState = getStrethRectState(event->pos());
	if (event->button() == Qt::LeftButton)
	{
		if (m_currentCaptureState == InitCapture)
		{
			m_currentCaptureState = BeginCaptureImage;
			m_beginPoint = event->pos();
		}
		// 是否在拉伸的小矩形中;
		else if (m_stretchRectState != NotSelect)
		{
			m_currentCaptureState = BeginMoveStretchRect;
			// 当前鼠标在拖动选中区顶点时,设置鼠标当前状态;
			setStretchCursorStyle(m_stretchRectState);
			m_beginMovePoint = event->pos();
		}
		// 是否在选中的矩形中;
		else if (isPressPointInSelectRect(event->pos()))
		{
			m_currentCaptureState = BeginMoveCaptureArea;
			m_beginMovePoint = event->pos();
		}
	}
	return QWidget::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
	if (m_currentCaptureState == BeginCaptureImage)
	{
		m_endPoint = event->pos();
		update();
	}
	else if (m_currentCaptureState == BeginMoveCaptureArea) 
	{
		m_endMovePoint = event->pos();
		update();
	}
	else if (m_currentCaptureState == BeginMoveStretchRect)
	{
		m_endMovePoint = event->pos();
		update();
		// 当前鼠标在拖动选中区顶点时,在鼠标未停止移动前，一直保持鼠标当前状态;
		return QWidget::mouseMoveEvent(event);
	}


	// 根据鼠标是否在选中区域内设置鼠标样式;
	StretchRectState stretchRectState = getStrethRectState(event->pos());
	if (stretchRectState != NotSelect)
	{
		setStretchCursorStyle(stretchRectState);
	}
	else if (isPressPointInSelectRect(event->pos()))
	{
		setCursor(Qt::SizeAllCursor);
	}
	else if (!isPressPointInSelectRect(event->pos()) && m_currentCaptureState != BeginMoveCaptureArea)
	{
		setCursor(Qt::ArrowCursor);
	}

	return QWidget::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
	if (m_currentCaptureState == BeginCaptureImage)
	{
		m_currentCaptureState = FinishCaptureImage;
		m_endPoint = event->pos();
		update();
	}
	else if (m_currentCaptureState == BeginMoveCaptureArea)
	{
		m_currentCaptureState = FinishMoveCaptureArea;
		m_endMovePoint = event->pos();
		update();
	}
	else if (m_currentCaptureState == BeginMoveStretchRect)
	{
		m_currentCaptureState = FinishMoveStretchRect;
		m_endMovePoint = event->pos();
		update();
	}

	return QWidget::mouseReleaseEvent(event);
}

// 当前鼠标坐标是否在选取的矩形区域内;
bool MainWindow::isPressPointInSelectRect(QPoint mousePressPoint)
{
	QRect selectRect = getRect(m_beginPoint, m_endPoint);
	if (selectRect.contains(mousePressPoint))
	{
		return true;
	}

	return false;
}

void MainWindow::paintEvent(QPaintEvent *event)
{
	m_painter.begin(this);          //进行重绘;

	QColor shadowColor = QColor(0, 0, 0, 120);   //阴影颜色设置;
	m_painter.setPen(QPen(QColor(0 , 174 , 255), 1, Qt::SolidLine, Qt::FlatCap));    //设置画笔;
	m_painter.drawPixmap(0, 0, m_loadPixmap);                       //将背景图片画到窗体上;
	m_painter.fillRect(m_loadPixmap.rect(), shadowColor);
	drawImage();
	//画影罩效果;
	switch (m_currentCaptureState)
	{
	case InitCapture:
		break;
	case BeginCaptureImage:
	case FinishCaptureImage:
	case BeginMoveCaptureArea:
	case FinishMoveCaptureArea:
	case BeginMoveStretchRect:
	case FinishMoveStretchRect:
		m_currentSelectRect = getSelectRect();
		drawCaptureImage();
	default:
		break;
	}

	m_painter.end();  //重绘结束;
}

void MainWindow::drawImage(){

	QStringList sl = picPath.split(".");
	QString pre = sl[0];
	QStringList sll = pre.split("/");
	pre = sll[sll.size() - 1];
	QString path = txtPath + pre + ".txt";
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qDebug() << "Can't open the file!" << endl;
	}
	while (!file.atEnd()) {
		QByteArray line = file.readLine();
		QString str(line);
		QStringList sections = str.split(" "); //把每一个块装进一个QStringList中
		if (sections.size()>0){
			int x = sections[0].toInt();
			int y = sections[1].toInt();
			int width = sections[2].toInt();
			int height = sections[3].toInt();
			m_painter.drawRect(x, y, width, height);
			qDebug() << str;
		}
	}
}
// 根据当前截取状态获取当前选中的截图区域;
QRect MainWindow::getSelectRect()
{
	if (m_currentCaptureState == BeginCaptureImage || m_currentCaptureState == FinishCaptureImage)
	{
		return getRect(m_beginPoint, m_endPoint);
	}
	else if (m_currentCaptureState == BeginMoveCaptureArea || m_currentCaptureState == FinishMoveCaptureArea)
	{
		return getMoveRect();
	}
	else if (m_currentCaptureState == BeginMoveStretchRect || m_currentCaptureState == FinishMoveStretchRect)
	{
		return getStretchRect();
	}

	return QRect(0, 0, 0, 0);
}

// 绘制当前选中的截图区域;

void MainWindow::drawCaptureImage()
{
	m_capturePixmap = m_loadPixmap.copy(m_currentSelectRect);
	m_painter.drawPixmap(m_currentSelectRect.topLeft(), m_capturePixmap);
	m_painter.drawRect(m_currentSelectRect);
	drawStretchRect();
}

// 绘制选中矩形各拖拽点小矩形;
void MainWindow::drawStretchRect()
{
	QColor color = QColor(0, 174, 255);
	// 四个角坐标;
	QPoint topLeft = m_currentSelectRect.topLeft();
	QPoint topRight = m_currentSelectRect.topRight();
	QPoint bottomLeft = m_currentSelectRect.bottomLeft();
	QPoint bottomRight = m_currentSelectRect.bottomRight();
	// 四条边中间点坐标;
	QPoint leftCenter = QPoint(topLeft.x(), (topLeft.y() + bottomLeft.y()) / 2);
	QPoint topCenter = QPoint((topLeft.x() + topRight.x()) / 2, topLeft.y());
	QPoint rightCenter = QPoint(topRight.x(), leftCenter.y());
	QPoint bottomCenter = QPoint(topCenter.x(), bottomLeft.y());

	m_topLeftRect = QRect(topLeft.x() - STRETCH_RECT_WIDTH / 2, topLeft.y() - STRETCH_RECT_HEIGHT / 2, STRETCH_RECT_WIDTH, STRETCH_RECT_HEIGHT);
	m_topRightRect = QRect(topRight.x() - STRETCH_RECT_WIDTH / 2, topRight.y() - STRETCH_RECT_HEIGHT / 2, STRETCH_RECT_WIDTH, STRETCH_RECT_HEIGHT);
	m_bottomLeftRect = QRect(bottomLeft.x() - STRETCH_RECT_WIDTH / 2, bottomLeft.y() - STRETCH_RECT_HEIGHT / 2, STRETCH_RECT_WIDTH, STRETCH_RECT_HEIGHT);
	m_bottomRightRect = QRect(bottomRight.x() - STRETCH_RECT_WIDTH / 2, bottomRight.y() - STRETCH_RECT_HEIGHT / 2, STRETCH_RECT_WIDTH, STRETCH_RECT_HEIGHT);

	m_leftCenterRect = QRect(leftCenter.x() - STRETCH_RECT_WIDTH / 2, leftCenter.y() - STRETCH_RECT_HEIGHT / 2, STRETCH_RECT_WIDTH, STRETCH_RECT_HEIGHT);
	m_topCenterRect = QRect(topCenter.x() - STRETCH_RECT_WIDTH / 2, topCenter.y() - STRETCH_RECT_HEIGHT / 2, STRETCH_RECT_WIDTH, STRETCH_RECT_HEIGHT);
	m_rightCenterRect = QRect(rightCenter.x() - STRETCH_RECT_WIDTH / 2, rightCenter.y() - STRETCH_RECT_HEIGHT / 2, STRETCH_RECT_WIDTH, STRETCH_RECT_HEIGHT);
	m_bottomCenterRect = QRect(bottomCenter.x() - STRETCH_RECT_WIDTH / 2, bottomCenter.y() - STRETCH_RECT_HEIGHT / 2, STRETCH_RECT_WIDTH, STRETCH_RECT_HEIGHT);

	m_painter.fillRect(m_topLeftRect, color);
	m_painter.fillRect(m_topRightRect, color);
	m_painter.fillRect(m_bottomLeftRect, color);
	m_painter.fillRect(m_bottomRightRect, color);
	m_painter.fillRect(m_leftCenterRect, color);
	m_painter.fillRect(m_topCenterRect, color);
	m_painter.fillRect(m_rightCenterRect, color);
	m_painter.fillRect(m_bottomCenterRect, color);
}

//QKeyEvent *event
void MainWindow::keyPressEvent(QKeyEvent *event)
{
	// Esc 键退出截图;
	if (event->key() == Qt::Key_Escape){
		QMessageBox message_cd(QMessageBox::NoIcon, tr("警告"), tr("您是否要关闭此软件？"));
		message_cd.addButton(QString::fromUtf8("是"), QMessageBox::AcceptRole);
		message_cd.addButton(QString::fromUtf8("否"), QMessageBox::RejectRole);
		message_cd.move(this->width() / 2, this->height() / 2);
		if (message_cd.exec() == QMessageBox::AcceptRole)
		{
			close();
		}
	}
	// Eeter键完成截图;
	if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
	{

		QDateTime time = QDateTime::currentDateTime();
		QString str = time.toString("yyyyMMddhhmmss");

		//QString savePath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
		QString s = QString::number(count++, 10);
		QStringList sl = picPath.split(".");
		QString pre = sl[0];
		QStringList sll = pre.split("/");
		pre = sll[sll.size() - 1];
		m_capturePixmap.save(shotPath +pre+"_"+s + ".jpg");

		QClipboard *clipboard = QApplication::clipboard();
		clipboard->setImage(m_capturePixmap.toImage());

		QString fileName = txtPath + pre + ".txt";

		QFile file(fileName);
		if(!file.open(QIODevice::WriteOnly  | QIODevice::Text|QIODevice::Append))
		{
			 QMessageBox::warning(this,"sdf","can't open",QMessageBox::Yes);
		}
		QTextStream in(&file);
		int left_x = min(m_beginPoint.x(), m_endPoint.x());
		int left_y = min(m_beginPoint.y(), m_endPoint.y());
	    int pic_width, pic_height;
		if (left_x == m_beginPoint.x()){
			pic_width = m_endPoint.x() - m_beginPoint.x();
			pic_height = m_endPoint.y() - m_beginPoint.y();
		}
		else{
			pic_width = m_beginPoint.x() - m_endPoint.x();
			pic_height = m_beginPoint.y() - m_endPoint.y();
		}
		//left_x = left_x*scale_x;
		//left_y = left_y*scale_y;
		//pic_width = (int)(pic_width*scale_x);
		//pic_height = (int)(pic_height*scale_y);
		//in << m_beginPoint.x() << " " << m_beginPoint.y() << " " << m_endPoint.x() << " " << m_endPoint.y() << "\n";
		//in << m_beginPoint.x() << " " << m_beginPoint.y() << " " << m_endPoint.x() << " "<<m_endPoint.y() << "\n";
		in<<left_x<<" "<<left_y<<" "<<pic_width<<" "<<pic_height<<"\n";
		file.close();
		
		//close();
		m_currentCaptureState = InitCapture;
	}
}

// 根据beginPoint , endPoint 获取当前选中的矩形;
QRect MainWindow::getRect(const QPoint &beginPoint, const QPoint &endPoint)
{
	int x, y, width, height;
	width = qAbs(beginPoint.x() - endPoint.x());
	height = qAbs(beginPoint.y() - endPoint.y());
	x = beginPoint.x() < endPoint.x() ? beginPoint.x() : endPoint.x();
	y = beginPoint.y() < endPoint.y() ? beginPoint.y() : endPoint.y();

	QRect selectedRect = QRect(x, y, width, height);
	// 避免宽或高为零时拷贝截图有误;
	// 可以看QQ截图，当选取截图宽或高为零时默认为2;
	if (selectedRect.width() == 0)
	{
		selectedRect.setWidth(1);
	}
	if (selectedRect.height() == 0)
	{
		selectedRect.setHeight(1);
	}

	return selectedRect;
}

// 获取移动后,当前选中的矩形;
QRect MainWindow::getMoveRect()
{
	// 通过getMovePoint方法先检查当前是否移动超出屏幕;
	QPoint movePoint = getMovePoint();
	QPoint beginPoint = m_beginPoint + movePoint;
	QPoint endPoint = m_endPoint + movePoint;
	// 结束移动选区时更新当前m_beginPoint , m_endPoint,防止下一次操作时截取的图片有问题;
	if (m_currentCaptureState == FinishMoveCaptureArea)
	{
		m_beginPoint = beginPoint;
		m_endPoint = endPoint;
		m_beginMovePoint = QPoint(0, 0);
		m_endMovePoint = QPoint(0, 0);
	}
	return getRect(beginPoint, endPoint);
}

QPoint MainWindow::getMovePoint()
{
	QPoint movePoint = m_endMovePoint - m_beginMovePoint;
	QRect currentRect = getRect(m_beginPoint, m_endPoint);
	// 检查当前是否移动超出屏幕;

	//移动选区是否超出屏幕左边界;
	if (currentRect.topLeft().x() + movePoint.x() < 0)
	{
		movePoint.setX(0 - currentRect.topLeft().x());
	}
	//移动选区是否超出屏幕上边界;
	if (currentRect.topLeft().y() + movePoint.y() < 0)
	{
		movePoint.setY(0 - currentRect.topLeft().y());
	}
	//移动选区是否超出屏幕右边界;
	if (currentRect.bottomRight().x() + movePoint.x() > m_screenwidth)
	{
		movePoint.setX(m_screenwidth - currentRect.bottomRight().x());
	}
	//移动选区是否超出屏幕下边界;
	if (currentRect.bottomRight().y() + movePoint.y() > m_screenheight)
	{
		movePoint.setY(m_screenheight - currentRect.bottomRight().y());
	}

	return movePoint;
}

// 获取当前鼠标位于哪一个拖拽顶点;
StretchRectState MainWindow::getStrethRectState(QPoint point)
{
	StretchRectState stretchRectState = NotSelect;
	if (m_topLeftRect.contains(point))
	{
		stretchRectState = TopLeftRect;
	}
	else if (m_topCenterRect.contains(point))
	{
		stretchRectState = TopCenterRect;
	}
	else if (m_topRightRect.contains(point))
	{
		stretchRectState = TopRightRect;
	}
	else if (m_rightCenterRect.contains(point))
	{
		stretchRectState = RightCenterRect;
	}
	else if (m_bottomRightRect.contains(point))
	{
		stretchRectState = BottomRightRect;
	}
	else if (m_bottomCenterRect.contains(point))
	{
		stretchRectState = BottomCenterRect;
	}
	else if (m_bottomLeftRect.contains(point))
	{
		stretchRectState = BottomLeftRect;
	}
	else if (m_leftCenterRect.contains(point))
	{
		stretchRectState = LeftCenterRect;
	}

	return stretchRectState;
}

// 设置鼠标停在拖拽定点处的样式;

void MainWindow::setStretchCursorStyle(StretchRectState stretchRectState)
{
	switch (stretchRectState)
	{
	case NotSelect:
		setCursor(Qt::ArrowCursor);
		break;
	case TopLeftRect:
	case BottomRightRect:
		setCursor(Qt::SizeFDiagCursor);
		break;
	case TopRightRect:
	case BottomLeftRect:
		setCursor(Qt::SizeBDiagCursor);
		break;
	case LeftCenterRect:
	case RightCenterRect:
		setCursor(Qt::SizeHorCursor);
		break;
	case TopCenterRect:
	case BottomCenterRect:
		setCursor(Qt::SizeVerCursor);
		break;
	default:
		break;
	}
}

// 获取拖拽后的矩形选中区域;
QRect MainWindow::getStretchRect()
{
	QRect stretchRect;
	QRect currentRect = getRect(m_beginPoint, m_endPoint);
	switch (m_stretchRectState)
	{
	case NotSelect:
		stretchRect = getRect(m_beginPoint, m_endPoint);
		break;
	case TopLeftRect:
		{
			stretchRect = getRect(currentRect.bottomRight(), m_endMovePoint);
		}
		break;
	case TopRightRect:
		{
			QPoint beginPoint = QPoint(currentRect.topLeft().x(), m_endMovePoint.y());
			QPoint endPoint = QPoint(m_endMovePoint.x(), currentRect.bottomRight().y());
			stretchRect = getRect(beginPoint, endPoint);
		}
		break;
	case BottomLeftRect:
		{
			QPoint beginPoint = QPoint(m_endMovePoint.x() , currentRect.topLeft().y());
			QPoint endPoint = QPoint(currentRect.bottomRight().x(), m_endMovePoint.y());
			stretchRect = getRect(beginPoint, endPoint);
		}
		break;
	case BottomRightRect:
		{
			stretchRect = getRect(currentRect.topLeft(), m_endMovePoint);
		}
		break;
	case LeftCenterRect:
		{
			QPoint beginPoint = QPoint(m_endMovePoint.x(), currentRect.topLeft().y());
			stretchRect = getRect(beginPoint, currentRect.bottomRight());
		}
		break;
	case TopCenterRect:
	{
		QPoint beginPoint = QPoint(currentRect.topLeft().x(), m_endMovePoint.y());
		stretchRect = getRect(beginPoint, currentRect.bottomRight());
	}
		break;
	case RightCenterRect:
	{
		QPoint endPoint = QPoint(m_endMovePoint.x(), currentRect.bottomRight().y());
		stretchRect = getRect(currentRect.topLeft(), endPoint);
	}
		break;
	case BottomCenterRect:
	{
		QPoint endPoint = QPoint(currentRect.bottomRight().x(), m_endMovePoint.y());
		stretchRect = getRect(currentRect.topLeft(), endPoint);
	}
		break;
	default:
	{
		stretchRect = getRect(m_beginPoint , m_endPoint );
	}
		break;
	}

	// 拖动结束更新 m_beginPoint , m_endPoint;
	if (m_currentCaptureState == FinishMoveStretchRect)
	{
		m_beginPoint = stretchRect.topLeft();
		m_endPoint = stretchRect.bottomRight();
	}

	return stretchRect;
}
