/*
 * Stellarium
 * Copyright (C) 2007 Fabien Chereau
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA  02110-1335, USA.
 */

#include "StelMainView.hpp"
#include "StelApp.hpp"
#include "StelCore.hpp"
#include "StelFileMgr.hpp"
#include "StelProjector.hpp"
#include "StelModuleMgr.hpp"
#include "StelPainter.hpp"
#include "StelGui.hpp"
#include "StelTranslator.hpp"
#include "StelUtils.hpp"
#include "StelActionMgr.hpp"
#include "StelOpenGL.hpp"

#include <QDebug>
#include <QDir>
#ifdef USE_OLD_QGLWIDGET
#include <QGLWidget>
#else
#include <QOpenGLWidget>
#endif
#include <QApplication>
#include <QDesktopWidget>
#include <QGuiApplication>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsAnchorLayout>
#include <QGraphicsWidget>
#include <QGraphicsEffect>
#include <QFileInfo>
#include <QIcon>
#include <QMoveEvent>
#include <QPluginLoader>
#include <QScreen>
#include <QSettings>
#include <QtPlugin>
#include <QThread>
#include <QTimer>
#include <QWidget>
#include <QWindow>
#include <QMessageBox>
#include <QStandardPaths>
#ifdef Q_OS_WIN
	#include <QPinchGesture>
#endif
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>
#include <QOpenGLPaintDevice>
#ifdef OPENGL_DEBUG_LOGGING
#include <QOpenGLDebugLogger>
#endif

#include <clocale>

// Initialize static variables
StelMainView* StelMainView::singleton = NULL;

#ifdef USE_OLD_QGLWIDGET
class StelGLWidget : public QGLWidget
#else
class StelGLWidget : public QOpenGLWidget
#endif
{
public:
	StelGLWidget(const QSurfaceFormat& fmt, StelMainView* parent)
		:
#ifdef USE_OLD_QGLWIDGET
		  QGLWidget(QGLFormat::fromSurfaceFormat(fmt),parent),
#else
		  QOpenGLWidget(parent),
#endif
		  parent(parent),
		  initialized(false)
	{
		qDebug()<<"StelGLWidget constructor";
#ifndef USE_OLD_QGLWIDGET
		setFormat(fmt);
#endif

		//because we always draw the full background,
		//lets skip drawing the system background
		setAttribute(Qt::WA_OpaquePaintEvent);
		setAutoFillBackground(false);
	}

	~StelGLWidget()
	{
		qDebug()<<"StelGLWidget destroyed";
	}

	virtual void initializeGL() Q_DECL_OVERRIDE
	{
		if(initialized)
		{
			qWarning()<<"Double initialization, should not happen";
			Q_ASSERT(false);
			return;
		}

		//This seems to be the correct place to initialize all
		//GL related stuff of the application
		//this includes all the init() calls of the modules

#ifdef USE_OLD_QGLWIDGET
		QOpenGLContext* ctx = context()->contextHandle();
#else
		QOpenGLContext* ctx = context();
#endif
		Q_ASSERT(ctx == QOpenGLContext::currentContext());
		StelOpenGL::mainContext = ctx; //throw an error when StelOpenGL functions are executed in another context

		qDebug()<<"initializeGL";
		qDebug() << "OpenGL supported version: " << QString((char*)ctx->functions()->glGetString(GL_VERSION));
		qDebug() << "Current Format: " << this->format();

		if (qApp->property("onetime_compat33")==true)
		{
			// This may not return the version number set previously!
			qDebug() << "StelGLWidget context format version:" << ctx->format().majorVersion() << "." << context()->format().minorVersion();
			qDebug() << "StelGLWidget has CompatibilityProfile:" << (ctx->format().profile()==QSurfaceFormat::CompatibilityProfile ? "yes" : "no") << "(" <<context()->format().profile() << ")";
		}

		parent->init();
		initialized = true;
	}

protected:
	virtual void paintGL() Q_DECL_OVERRIDE
	{
		//this is actually never called because the
		//QGraphicsView intercepts the paint event
		//we have to draw in the background of the scene
		//or as a QGraphicsItem
		qDebug()<<"paintGL";
	}
	virtual void resizeGL(int w, int h) Q_DECL_OVERRIDE
	{
		//we probably can ignore this method,
		//it seems it is also never called
		qDebug()<<"resizeGL"<<w<<h;
	}

private:
	StelMainView* parent;
	bool initialized;
};

// A custom QGraphicsEffect to apply the night mode on top of the screen.
class NightModeGraphicsEffect : public QGraphicsEffect
{
public:
	NightModeGraphicsEffect(StelMainView* parent = NULL)
		: QGraphicsEffect(parent),
		  parent(parent), fbo(NULL)
	{
		Q_ASSERT(parent->glContext() == QOpenGLContext::currentContext());

		program = new QOpenGLShaderProgram(this);
		QString vertexCode =
				"attribute highp vec4 a_pos;\n"
				"attribute highp vec2 a_texCoord;\n"
				"varying highp   vec2 v_texCoord;\n"
				"void main(void)\n"
				"{\n"
				"v_texCoord = a_texCoord;\n"
				"gl_Position = a_pos;\n"
				"}\n";
		QString fragmentCode =
				"varying highp vec2 v_texCoord;\n"
				"uniform sampler2D  u_source;\n"
				"void main(void)\n"
				"{\n"
				"	mediump vec3 color = texture2D(u_source, v_texCoord).rgb;\n"
				"	mediump float luminance = max(max(color.r, color.g), color.b);\n"
				"	gl_FragColor = vec4(luminance, 0.0, 0.0, 1.0);\n"
				"}\n";
		program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexCode);
		program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentCode);
		program->link();
		vars.pos = program->attributeLocation("a_pos");
		vars.texCoord = program->attributeLocation("a_texCoord");
		vars.source = program->uniformLocation("u_source");
	}

	virtual ~NightModeGraphicsEffect()
	{
		Q_ASSERT(parent->glContext() == QOpenGLContext::currentContext());
		//clean up fbo
		delete fbo;
	}
protected:
	virtual void draw(QPainter* painter) Q_DECL_OVERRIDE
	{
		Q_ASSERT(parent->glContext() == QOpenGLContext::currentContext());
		QOpenGLFunctions* gl = QOpenGLContext::currentContext()->functions();

		QPaintDevice* paintDevice = painter->device();

		int mainFBO;
		gl->glGetIntegerv(GL_FRAMEBUFFER_BINDING, &mainFBO);

		int pixelRatio = paintDevice->devicePixelRatio();
		QSize size(paintDevice->width() * pixelRatio, paintDevice->height() * pixelRatio);
		if (fbo && fbo->size() != size)
		{
			delete fbo;
			fbo = NULL;
		}
		if (!fbo)
		{
			QOpenGLFramebufferObjectFormat format;
			format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
			format.setInternalTextureFormat(GL_RGBA);
			fbo = new QOpenGLFramebufferObject(size, format);
		}

		// we have to use our own paint device
		// we need this because using the original paint device (QOpenGLWidgetPaintDevice when used with QOpenGLWidget) will rebind the default FBO randomly
		// but using 2 GL painters at the same time can mess up the GL state, so we should close the old one first

		// stop drawing to the old paint device to make sure state is reset correctly
		painter->end();

		// create our paint device
		QOpenGLPaintDevice fboPaintDevice(size);
		fboPaintDevice.setDevicePixelRatio(pixelRatio);

		fbo->bind();
		painter->begin(&fboPaintDevice);
		drawSource(painter);
		painter->end();

		painter->begin(paintDevice);

		//painter->beginNativePainting();
		program->bind();
		const GLfloat pos[] = {-1, -1, +1, -1, -1, +1, +1, +1};
		const GLfloat texCoord[] = {0, 0, 1, 0, 0, 1, 1, 1};
		program->setUniformValue(vars.source, 0);
		program->setAttributeArray(vars.pos, pos, 2);
		program->setAttributeArray(vars.texCoord, texCoord, 2);
		program->enableAttributeArray(vars.pos);
		program->enableAttributeArray(vars.texCoord);
		gl->glBindTexture(GL_TEXTURE_2D, fbo->texture());
		gl->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		program->release();
		//painter->endNativePainting();
	}

private:
	StelMainView* parent;
	QOpenGLFramebufferObject* fbo;
	QOpenGLShaderProgram *program;
	struct {
		int pos;
		int texCoord;
		int source;
	} vars;
};

class StelGraphicsScene : public QGraphicsScene
{
public:
	StelGraphicsScene(StelMainView* parent)
		: QGraphicsScene(parent), parent(parent)
	{
		qDebug()<<"StelGraphicsScene constructor";
	}

protected:

	void keyPressEvent(QKeyEvent* event) Q_DECL_OVERRIDE
	{
		// Try to trigger a global shortcut.
		StelActionMgr* actionMgr = StelApp::getInstance().getStelActionManager();
		if (actionMgr->pushKey(event->key() + event->modifiers(), true)) {
			event->setAccepted(true);
			parent->thereWasAnEvent(); // Refresh screen ASAP
			return;
		}
		//pass event on to items otherwise
		QGraphicsScene::keyPressEvent(event);
	}

private:
	StelMainView* parent;
};

class StelRootItem : public QGraphicsObject
{
public:
	StelRootItem(StelMainView* mainView, QGraphicsItem* parent = NULL)
		: QGraphicsObject(parent), mainView(mainView)
	{
		setFlag(QGraphicsItem::ItemClipsToShape);
		setFlag(QGraphicsItem::ItemClipsChildrenToShape);
		setFlag(QGraphicsItem::ItemIsFocusable);

		setAcceptHoverEvents(true);

#ifdef Q_OS_WIN
		setAcceptTouchEvents(true);
		grabGesture(Qt::PinchGesture);
#endif
		setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);
		previousPaintTime = StelApp::getTotalRunTime();
	}

	void setSize(const QSize& size)
	{
		prepareGeometryChange();
		rect.setSize(size);
	}

protected:
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE
	{
		Q_UNUSED(option);
		Q_UNUSED(widget);

		//a sanity check
		Q_ASSERT(mainView->glContext() == QOpenGLContext::currentContext());

		const double now = StelApp::getTotalRunTime();
		double dt = now - previousPaintTime;
		//qDebug()<<"dt"<<dt;
		previousPaintTime = now;

		//important to call this, or Qt may have invalid state after we have drawn (wrong textures, etc...)
		painter->beginNativePainting();

		//fix for bug 1628072 caused by QTBUG-56798
#ifndef QT_NO_DEBUG
		StelOpenGL::clearGLErrors();
#endif


		QOpenGLFunctions* gl = QOpenGLContext::currentContext()->functions();

		//clear the buffer (not strictly required for us because we repaint all pixels, but should improve perf on tile-based renderers)
		gl->glClearColor(0,0,0,0); //we also clear alpha to zero
		gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		//update and draw
		StelApp& app = StelApp::getInstance();
		app.update(dt); // may also issue GL calls
		app.draw();
		painter->endNativePainting();

		mainView->drawEnded();
	}

	virtual QRectF boundingRect() const Q_DECL_OVERRIDE
	{
		return rect;
	}

	//*** Main event handlers to pass on to StelApp ***//
	void mousePressEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE
	{
		QMouseEvent ev = convertMouseEvent(event);
		StelApp::getInstance().handleClick(&ev);
		event->setAccepted(ev.isAccepted());
		if(ev.isAccepted())
			mainView->thereWasAnEvent();
	}

	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE
	{
		QMouseEvent ev = convertMouseEvent(event);
		StelApp::getInstance().handleClick(&ev);
		event->setAccepted(ev.isAccepted());
		if(ev.isAccepted())
			mainView->thereWasAnEvent();
	}

	void mouseMoveEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE
	{
		QMouseEvent ev = convertMouseEvent(event);
		QPointF pos = ev.pos();
		event->setAccepted(StelApp::getInstance().handleMove(pos.x(), pos.y(), ev.buttons()));
		if(event->isAccepted())
			mainView->thereWasAnEvent();
	}

	void wheelEvent(QGraphicsSceneWheelEvent *event) Q_DECL_OVERRIDE
	{
		QPointF pos = event->scenePos();
		pos.setY(rect.height() - 1 - pos.y());
		QWheelEvent newEvent(QPoint(pos.x(),pos.y()), event->delta(), event->buttons(), event->modifiers(), event->orientation());
		StelApp::getInstance().handleWheel(&newEvent);
		event->setAccepted(newEvent.isAccepted());
		if(newEvent.isAccepted())
			mainView->thereWasAnEvent();
	}

	void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE
	{
		StelApp::getInstance().handleKeys(event);
		if(event->isAccepted())
			mainView->thereWasAnEvent();
	}

	void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE
	{
		StelApp::getInstance().handleKeys(event);
		if(event->isAccepted())
			mainView->thereWasAnEvent();
	}

	//*** Gesture and touch support, currently only for Windows
#ifdef Q_OS_WIN
	bool event(QEvent * e) Q_DECL_OVERRIDE
	{
		switch (e->type()){
			case QEvent::TouchBegin:
			case QEvent::TouchUpdate:
			case QEvent::TouchEnd:
			{
				QTouchEvent *touchEvent = static_cast<QTouchEvent *>(e);
				QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();

				if (touchPoints.count() == 1)
					setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);

				return true;
				break;
			}

			case QEvent::Gesture:
				setAcceptedMouseButtons(0);
				return gestureEvent(static_cast<QGestureEvent*>(e));
				break;

			default:
				return QGraphicsObject::event(e);
		}
	}

private:
	bool gestureEvent(QGestureEvent *event)
	{
		if (QGesture *pinch = event->gesture(Qt::PinchGesture))
			pinchTriggered(static_cast<QPinchGesture *>(pinch));

		return true;
	}

	void pinchTriggered(QPinchGesture *gesture)
	{
		QPinchGesture::ChangeFlags changeFlags = gesture->changeFlags();
		if (changeFlags & QPinchGesture::ScaleFactorChanged) {
			qreal zoom = gesture->scaleFactor();

			if (zoom < 2 && zoom > 0.5){
				StelApp::getInstance().handlePinch(zoom, true);
			}
		}
	}
#endif

private:
	//! Helper function to convert a QGraphicsSceneMouseEvent to a QMouseEvent suitable for StelApp consumption
	QMouseEvent convertMouseEvent(QGraphicsSceneMouseEvent *event) const
	{
		//convert graphics scene mouse event to widget style mouse event
		QEvent::Type t = QEvent::None;
		switch(event->type())
		{
			case QEvent::GraphicsSceneMousePress:
				t = QEvent::MouseButtonPress;
				break;
			case QEvent::GraphicsSceneMouseRelease:
				t = QEvent::MouseButtonRelease;
				break;
			case QEvent::GraphicsSceneMouseMove:
				t = QEvent::MouseMove;
				break;
			case QEvent::GraphicsSceneMouseDoubleClick:
				//note: the old code seems to have ignored double clicks
				// and handled them the same as normal mouse presses
				//if we ever want to handle double clicks, switch out these lines
				//t = QEvent::MouseButtonDblClick;
				t = QEvent::MouseButtonPress;
				break;
			default:
				//warn in release and assert in debug
				qWarning("Unhandled mouse event type %d",event->type());
				Q_ASSERT(false);
		}

		QPointF pos = event->scenePos();
		//Y needs to be inverted
		pos.setY(rect.height() - 1 - pos.y());
		return QMouseEvent(t,pos,event->button(),event->buttons(),event->modifiers());
	}

	QRectF rect;
	double previousPaintTime;
	StelMainView* mainView;
};

//! Initialize and render Stellarium gui.
class StelGuiItem : public QGraphicsWidget
{
public:
	StelGuiItem(QGraphicsItem* parent = NULL)
		: QGraphicsWidget(parent)
	{
		StelApp::getInstance().getGui()->init(this);
	}

protected:
	void resizeEvent(QGraphicsSceneResizeEvent* event) Q_DECL_OVERRIDE
	{
		Q_UNUSED(event);
		//widget->setGeometry(0, 0, size().width(), size().height());
		StelApp::getInstance().getGui()->forceRefreshGui();
	}
private:
	//QGraphicsWidget *widget;
	// void onSizeChanged();
};

StelMainView::StelMainView(QSettings* settings)
	: QGraphicsView(),
	  guiItem(NULL),
	  gui(NULL),
	  stelApp(NULL),
	  updateQueued(false),
	  flagInvertScreenShotColors(false),
	  flagOverwriteScreenshots(false),
	  screenShotPrefix("stellarium-"),
	  screenShotDir(""),
	  cursorTimeout(-1.f), flagCursorTimeout(false), maxfps(10000.f)
{
	setAttribute(Qt::WA_OpaquePaintEvent);
	setAutoFillBackground(false);

	configuration = settings;
	StelApp::initStatic();

	minFpsTimer = new QTimer(this);
	minFpsTimer->setTimerType(Qt::PreciseTimer);
	minFpsTimer->setInterval(1000/minfps);
	connect(minFpsTimer,SIGNAL(timeout()),this,SLOT(minFPSUpdate()));
	
	// Can't create 2 StelMainView instances
	Q_ASSERT(!singleton);
	singleton = this;

	setWindowIcon(QIcon(":/mainWindow/icon.bmp"));
	initTitleI18n();
	setObjectName("Mainview");

	setViewportUpdateMode(QGraphicsView::NoViewportUpdate);
	setFrameShape(QFrame::NoFrame);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	//because we only want child elements to have focus, we turn it off here
	setFocusPolicy(Qt::NoFocus);
	connect(this, SIGNAL(screenshotRequested()), this, SLOT(doScreenshot()));

	lastEventTimeSec = 0;

#ifdef OPENGL_DEBUG_LOGGING
	glLogger = new QOpenGLDebugLogger(this);
	connect(glLogger, SIGNAL(messageLogged(QOpenGLDebugMessage)), this, SLOT(logGLMessage(QOpenGLDebugMessage)));
#endif

	//get the desired opengl format parameters
	QSurfaceFormat glFormat = getDesiredGLFormat();
	// VSync control
	QVariant vsync = configuration->value("video/vsync");
	if(vsync.isValid() && vsync.canConvert<bool>()) // if the config parameter is not set we use system default (which should be true)
		glFormat.setSwapInterval(vsync.toBool());

	qDebug()<<"Desired surface format: "<<glFormat;

#if QT_VERSION >= QT_VERSION_CHECK(5,4,0)
	//we set the default format to our required format, if possible
	//this only works with Qt 5.4+
	QSurfaceFormat defFmt = glFormat;
	//we don't need these buffers in the background
	defFmt.setAlphaBufferSize(0);
	defFmt.setStencilBufferSize(0);
	defFmt.setDepthBufferSize(0);
	QSurfaceFormat::setDefaultFormat(defFmt);
#endif

	//QGLWidget should set the format in constructor to prevent creating an unnecessary temporary context
	glWidget = new StelGLWidget(glFormat, this);
	setViewport(glWidget);

	stelScene = new StelGraphicsScene(this);
	setScene(stelScene);
	scene()->setItemIndexMethod(QGraphicsScene::NoIndex);
	rootItem = new StelRootItem(this);

	// Workaround (see Bug #940638) Although we have already explicitly set
	// LC_NUMERIC to "C" in main.cpp there seems to be a bug in OpenGL where
	// it will silently reset LC_NUMERIC to the value of LC_ALL during OpenGL
	// initialization. This has been observed on Ubuntu 11.10 under certain
	// circumstances, so here we set it again just to be on the safe side.
	setlocale(LC_NUMERIC, "C");
	// End workaround

#ifdef USE_OLD_QGLWIDGET
	// StelGLWidget::initializeGL is seemingly never called automatically with the QGLWidget, so we have to do it ourselves
	//we have to force context creation here
	glWidget->makeCurrent();
	glWidget->initializeGL();
#endif
}

void StelMainView::resizeEvent(QResizeEvent* event)
{
	if(scene())
	{
		const QSize& sz = event->size();
		scene()->setSceneRect(QRect(QPoint(0, 0), sz));
		rootItem->setSize(sz);
		if(guiItem)
			guiItem->setGeometry(QRectF(0.0f,0.0f,sz.width(),sz.height()));
	}
	QGraphicsView::resizeEvent(event);
}

void StelMainView::focusSky() {
	//scene()->setActiveWindow(0);
	rootItem->setFocus();
}

StelMainView::~StelMainView()
{
	//delete the night view graphic effect here while GL context is still valid
	rootItem->setGraphicsEffect(Q_NULLPTR);
	StelApp::deinitStatic();
}

QSurfaceFormat StelMainView::getDesiredGLFormat() const
{
#if QT_VERSION < QT_VERSION_CHECK(5,4,0)
	//default-constructed format
	QSurfaceFormat fmt;
#else
	//use the default format as basis
	QSurfaceFormat fmt = QSurfaceFormat::defaultFormat();
	qDebug()<<"Default surface format: "<<fmt;
#endif

	//if on an GLES build, do not set the format
#ifndef QT_OPENGL_ES_2
	// OGL 2.1 + FBOs should basically be the minimum required for Stellarium
	fmt.setRenderableType(QSurfaceFormat::OpenGL);
	fmt.setMajorVersion(2);
	fmt.setMinorVersion(1);

	// The following is NOT needed (or even supported) when we request a 2.1 context
	// The implementation may give us a newer context,
	// but compatibility with 2.1 should be ensured automatically
	//fmt.setProfile(QSurfaceFormat::CompatibilityProfile);
	//fmt.setOption(QSurfaceFormat::DeprecatedFunctions);
#endif

	//request some sane buffer formats
	fmt.setRedBufferSize(8);
	fmt.setGreenBufferSize(8);
	fmt.setBlueBufferSize(8);
	fmt.setAlphaBufferSize(8);
	fmt.setDepthBufferSize(24);
	//I dont think we use the stencil buffer for anything
	//but maybe Qt needs it
	fmt.setStencilBufferSize(8);

#ifdef OPENGL_DEBUG_LOGGING
	//try to enable GL debugging using GL_KHR_debug
	fmt.setOption(QSurfaceFormat::DebugContext);
#endif
	//vsync needs to be set on the default format for it to work
	//fmt.setSwapInterval(0);

	return fmt;
}

void StelMainView::init()
{
#ifdef OPENGL_DEBUG_LOGGING
	if(!QOpenGLContext::currentContext()->hasExtension(QByteArrayLiteral("GL_KHR_debug")))
		qWarning()<<"GL_KHR_debug extension missing, OpenGL debug logger will likely not work";
	if(glLogger->initialize())
	{
		qDebug()<<"OpenGL debug logger initialized";
		QVector<GLuint> disabledMsgs;
		//if your GL implementation spams some output you are not interested in,
		//you can disable their message IDs here
		//disabledMsgs.append(100);
		glLogger->disableMessages(disabledMsgs);
		glLogger->startLogging(QOpenGLDebugLogger::SynchronousLogging);
		//the internal log buffer may not be empty, so check it
		foreach(const QOpenGLDebugMessage& msg, glLogger->loggedMessages())
		{
			logGLMessage(msg);
		}
	}
	else
		qWarning()<<"Failed to initialize OpenGL debug logger";

	connect(QOpenGLContext::currentContext(),SIGNAL(aboutToBeDestroyed()),this,SLOT(contextDestroyed()));
	//for easier debugging, print the adress of the main GL context
	qDebug()<<"CurCtxPtr:"<<QOpenGLContext::currentContext();
#endif

	qDebug()<<"StelMainView::init";

	glInfo.mainContext = QOpenGLContext::currentContext();
	glInfo.functions = glInfo.mainContext->functions();
	glInfo.vendor = QString(reinterpret_cast<const char*>(glInfo.functions->glGetString(GL_VENDOR)));
	glInfo.renderer = QString(reinterpret_cast<const char*>(glInfo.functions->glGetString(GL_RENDERER)));

	gui = new StelGui();

	QSettings* conf = configuration;

	// Should be check of requirements disabled?
	if (conf->value("main/check_requirements", true).toBool())
	{
		// Find out lots of debug info about supported version of OpenGL and vendor/renderer.
		processOpenGLdiagnosticsAndWarnings(conf, QOpenGLContext::currentContext());
	}

	//create and initialize main app
	stelApp = new StelApp(this);
	stelApp->setGui(gui);

	stelApp->init(conf);
	//this makes sure the app knows how large the window is
	connect(stelScene,SIGNAL(sceneRectChanged(QRectF)),stelApp,SLOT(glWindowHasBeenResized(QRectF)));
	//also immediately set the current values
	stelApp->glWindowHasBeenResized(stelScene->sceneRect());

	StelActionMgr *actionMgr = stelApp->getStelActionManager();
	actionMgr->addAction("actionSave_Screenshot_Global", N_("Miscellaneous"), N_("Save screenshot"), this, "saveScreenShot()", "Ctrl+S");
	actionMgr->addAction("actionSet_Full_Screen_Global", N_("Display Options"), N_("Full-screen mode"), this, "fullScreen", "F11");
	
	StelPainter::initGLShaders();

	guiItem = new StelGuiItem(rootItem);
	scene()->addItem(rootItem);
	//set the default focus to the sky
	focusSky();
	nightModeEffect = new NightModeGraphicsEffect(this);
	updateNightModeProperty(StelApp::getInstance().getVisionModeNight());
	//install the effect on the whole view
	rootItem->setGraphicsEffect(nightModeEffect);

	QDesktopWidget *desktop = QApplication::desktop();
	int screen = conf->value("video/screen_number", 0).toInt();
	if (screen < 0 || screen >= desktop->screenCount())
	{
		qWarning() << "WARNING: screen" << screen << "not found";
		screen = 0;
	}
	QRect screenGeom = desktop->screenGeometry(screen);

	QSize size = QSize(conf->value("video/screen_w", screenGeom.width()).toInt(),
		     conf->value("video/screen_h", screenGeom.height()).toInt());

	bool fullscreen = conf->value("video/fullscreen", true).toBool();

	// Without this, the screen is not shown on a Mac + we should use resize() for correct work of fullscreen/windowed mode switch. --AW WTF???
	resize(size);

	if (fullscreen)
	{
		// The "+1" below is to work around Linux/Gnome problem with mouse focus.
		move(screenGeom.x()+1, screenGeom.y()+1);
		// The fullscreen window appears on screen where is the majority of
		// the normal window. Therefore we crop the normal window to the
		// screen area to ensure that the majority is not on another screen.
		setGeometry(geometry() & screenGeom);
		setFullScreen(true);
	}
	else
	{
		setFullScreen(false);
		int x = conf->value("video/screen_x", 0).toInt();
		int y = conf->value("video/screen_y", 0).toInt();
		move(x + screenGeom.x(), y + screenGeom.y());
	}

	flagInvertScreenShotColors = conf->value("main/invert_screenshots_colors", false).toBool();
	setFlagCursorTimeout(conf->value("gui/flag_mouse_cursor_timeout", false).toBool());
	setCursorTimeout(conf->value("gui/mouse_cursor_timeout", 10.f).toFloat());
	setMaxFps(conf->value("video/maximum_fps",10000.f).toFloat());
	setMinFps(conf->value("video/minimum_fps",10000.f).toFloat());

	// XXX: This should be done in StelApp::init(), unfortunately for the moment we need to init the gui before the
	// plugins, because the gui creates the QActions needed by some plugins.
	stelApp->initPlugIns();

	// The script manager can only be fully initialized after the plugins have loaded.
	stelApp->initScriptMgr();

	// Set the global stylesheet, this is only useful for the tooltips.
	StelGui* gui = dynamic_cast<StelGui*>(stelApp->getGui());
	if (gui!=NULL)
		setStyleSheet(gui->getStelStyle().qtStyleSheet);
	connect(stelApp, SIGNAL(visionNightModeChanged(bool)), this, SLOT(updateNightModeProperty(bool)));

	// I doubt this will have any effect on framerate, but may cause problems elsewhere?
	QThread::currentThread()->setPriority(QThread::HighestPriority);
#ifndef NDEBUG
	// Get an overview of module callOrders
	if (qApp->property("verbose")==true)
	{
		StelApp::getInstance().dumpModuleActionPriorities(StelModule::ActionDraw);
		StelApp::getInstance().dumpModuleActionPriorities(StelModule::ActionUpdate);
		StelApp::getInstance().dumpModuleActionPriorities(StelModule::ActionHandleMouseClicks);
		StelApp::getInstance().dumpModuleActionPriorities(StelModule::ActionHandleMouseMoves);
		StelApp::getInstance().dumpModuleActionPriorities(StelModule::ActionHandleKeys);
	}
#endif
}

void StelMainView::updateNightModeProperty(bool b)
{
	// So that the bottom bar tooltips get properly rendered in night mode.
	setProperty("nightMode", b);
	nightModeEffect->setEnabled(b);
}

// This is a series of various diagnostics based on "bugs" reported for 0.13.0 and 0.13.1.
// Almost all can be traced to insufficient driver level.
// No changes of OpenGL state is done here.
// If problems are detected, warn the user one time, but continue. Warning panel will be suppressed on next start.
// Work in progress, as long as we get reports about bad systems or until OpenGL startup is finalized and safe.
// Several tests do not apply to MacOS X.
void StelMainView::processOpenGLdiagnosticsAndWarnings(QSettings *conf, QOpenGLContext *context) const
{
#ifdef Q_OS_MAC
	Q_UNUSED(conf);
#endif
	QSurfaceFormat format=context->format();

	// These tests are not required on MacOS X
#ifndef Q_OS_MAC
	bool openGLerror=false;
	if (format.renderableType()==QSurfaceFormat::OpenGL || format.renderableType()==QSurfaceFormat::OpenGLES)
	{
		qDebug() << "Detected:" << (format.renderableType()==QSurfaceFormat::OpenGL  ? "OpenGL" : "OpenGL ES" ) << QString("%1.%2").arg(format.majorVersion()).arg(format.minorVersion());
	}
	else
	{
		openGLerror=true;
		qDebug() << "Neither OpenGL nor OpenGL ES detected: Unsupported Format!";
	}
#endif
	QOpenGLFunctions* gl = context->functions();

	QString glDriver(reinterpret_cast<const char*>(gl->glGetString(GL_VERSION)));
	qDebug() << "Driver version string:" << glDriver;
	qDebug() << "GL vendor is" << QString(reinterpret_cast<const char*>(gl->glGetString(GL_VENDOR)));
	QString glRenderer(reinterpret_cast<const char*>(gl->glGetString(GL_RENDERER)));
	qDebug() << "GL renderer is" << glRenderer;

	// Minimal required version of OpenGL for Qt5 is 2.1 and OpenGL Shading Language may be 1.20 (or OpenGL ES is 2.0 and GLSL ES is 1.0).
	// As of V0.13.0..1, we use GLSL 1.10/GLSL ES 1.00 (implicitly, by omitting a #version line), but in case of using ANGLE we need hardware
	// detected as providing ps_3_0.
	// This means, usually systems with OpenGL3 support reported in the driver will work, those with reported 2.1 only will almost certainly fail.
	// If platform does not even support minimal OpenGL version for Qt5, then tell the user about troubles and quit from application.
	// This test is apparently not applicable on MacOS X due to its behaving differently from all other known OSes.
	// The correct way to handle driver issues on MacOS X remains however unclear for now.
#ifndef Q_OS_MAC
	bool isMesa=glDriver.contains("Mesa", Qt::CaseInsensitive);
	#ifdef Q_OS_WIN
	bool isANGLE=glRenderer.startsWith("ANGLE", Qt::CaseSensitive);
	#endif
	if ( openGLerror ||
	     ((format.renderableType()==QSurfaceFormat::OpenGL  ) && (format.version() < QPair<int, int>(2, 1)) && !isMesa) ||
	     ((format.renderableType()==QSurfaceFormat::OpenGL  ) && (format.version() < QPair<int, int>(2, 0)) &&  isMesa) || // Mesa defaults to 2.0 but works!
	     ((format.renderableType()==QSurfaceFormat::OpenGLES) && (format.version() < QPair<int, int>(2, 0)))  )
	{
		#ifdef Q_OS_WIN
		if ((!isANGLE) && (!isMesa))
			qWarning() << "Oops... Insufficient OpenGL version. Please update drivers, graphics hardware, or use --angle-mode (or even --mesa-mode) option.";
		else if (isANGLE)
			qWarning() << "Oops... Insufficient OpenGLES version in ANGLE. Please update drivers, graphics hardware, or use --mesa-mode option.";
		else
			qWarning() << "Oops... Insufficient OpenGL version. Mesa failed! Please send a bug report.";

		QMessageBox::critical(0, "Stellarium", q_("Insufficient OpenGL version. Please update drivers, graphics hardware, or use --angle-mode (or --mesa-mode) option."), QMessageBox::Abort, QMessageBox::Abort);
		#else
		qWarning() << "Oops... Insufficient OpenGL version. Please update drivers, or graphics hardware.";
		QMessageBox::critical(0, "Stellarium", q_("Insufficient OpenGL version. Please update drivers, or graphics hardware."), QMessageBox::Abort, QMessageBox::Abort);
		#endif
		exit(1);
	}
#endif
	// This call requires OpenGL2+.
	QString glslString(reinterpret_cast<const char*>(gl->glGetString(GL_SHADING_LANGUAGE_VERSION)));
	qDebug() << "GL Shading Language version is" << glslString;

	// Only give extended info if called on command line, for diagnostic.
	if (qApp->property("dump_OpenGL_details").toBool())
		dumpOpenGLdiagnostics();

#ifdef Q_OS_WIN
	// If we have ANGLE, check esp. for insufficient ps_2 level.
	if (isANGLE)
	{
		QRegExp angleVsPsRegExp(" vs_(\\d)_(\\d) ps_(\\d)_(\\d)");
		int angleVSPSpos=angleVsPsRegExp.indexIn(glRenderer);

		if (angleVSPSpos >-1)
		{
			float vsVersion=angleVsPsRegExp.cap(1).toFloat() + 0.1*angleVsPsRegExp.cap(2).toFloat();
			float psVersion=angleVsPsRegExp.cap(3).toFloat() + 0.1*angleVsPsRegExp.cap(4).toFloat();
			qDebug() << "VS Version Number detected: " << vsVersion;
			qDebug() << "PS Version Number detected: " << psVersion;
			if ((vsVersion<2.0) || (psVersion<3.0))
			{
				openGLerror=true;
				qDebug() << "This is not enough: we need DirectX9 with vs_2_0 and ps_3_0 or later.";
				qDebug() << "You should update graphics drivers, graphics hardware, or use the --mesa-mode option.";
				qDebug() << "Else, please try to use an older version like 0.12.5, and try with --safe-mode";

				if (conf->value("main/ignore_opengl_warning", false).toBool())
				{
					qDebug() << "Config option main/ignore_opengl_warning found, continuing. Expect problems.";
				}
				else
				{
					qDebug() << "You can try to run in an unsupported degraded mode by ignoring the warning and continuing.";
					qDebug() << "But more than likely problems will persist.";
					QMessageBox::StandardButton answerButton=
					QMessageBox::critical(0, "Stellarium", q_("Your DirectX/OpenGL ES subsystem has problems. See log for details.\nIgnore and suppress this notice in the future and try to continue in degraded mode anyway?"),
							      QMessageBox::Ignore|QMessageBox::Abort, QMessageBox::Abort);
					if (answerButton == QMessageBox::Abort)
					{
						qDebug() << "Aborting due to ANGLE OpenGL ES / DirectX vs or ps version problems.";
						exit(1);
					}
					else
					{
						qDebug() << "Ignoring all warnings, continuing without further question.";
						conf->setValue("main/ignore_opengl_warning", true);
					}
				}
			}
			else
				qDebug() << "vs/ps version is fine, we should not see a graphics problem.";
		}
		else
		{
			qDebug() << "Cannot parse ANGLE shader version string. This may indicate future problems.";
			qDebug() << "Please send a bug report that includes this log file and states if Stellarium runs or has problems.";
		}
	}
#endif
#ifndef Q_OS_MAC
	// Do a similar test for MESA: Ensure we have at least Mesa 10, Mesa 9 on FreeBSD (used for hardware-acceleration of AMD IGP) was reported to lose the stars.
	if (isMesa)
	{
		QRegExp mesaRegExp("Mesa (\\d+\\.\\d+)"); // we need only major version. Minor should always be here. Test?
		int mesaPos=mesaRegExp.indexIn(glDriver);

		if (mesaPos >-1)
		{
			float mesaVersion=mesaRegExp.cap(1).toFloat();
			qDebug() << "MESA Version Number detected: " << mesaVersion;
			if ((mesaVersion<10.0f))
			{
				openGLerror=true;
				qDebug() << "This is not enough: we need Mesa 10.0 or later.";
				qDebug() << "You should update graphics drivers or graphics hardware.";
				qDebug() << "Else, please try to use an older version like 0.12.5, and try there with --safe-mode";

				if (conf->value("main/ignore_opengl_warning", false).toBool())
				{
					qDebug() << "Config option main/ignore_opengl_warning found, continuing. Expect problems.";
				}
				else
				{
					qDebug() << "You can try to run in an unsupported degraded mode by ignoring the warning and continuing.";
					qDebug() << "But more than likely problems will persist.";
					QMessageBox::StandardButton answerButton=
					QMessageBox::critical(0, "Stellarium", q_("Your OpenGL/Mesa subsystem has problems. See log for details.\nIgnore and suppress this notice in the future and try to continue in degraded mode anyway?"),
							      QMessageBox::Ignore|QMessageBox::Abort, QMessageBox::Abort);
					if (answerButton == QMessageBox::Abort)
					{
						qDebug() << "Aborting due to OpenGL/Mesa insufficient version problems.";
						exit(1);
					}
					else
					{
						qDebug() << "Ignoring all warnings, continuing without further question.";
						conf->setValue("main/ignore_opengl_warning", true);
					}
				}
			}
			else
				qDebug() << "Mesa version is fine, we should not see a graphics problem.";
		}
		else
		{
			qDebug() << "Cannot parse Mesa Driver version string. This may indicate future problems.";
			qDebug() << "Please send a bug report that includes this log file and states if Stellarium runs or has problems.";
		}
	}
#endif

	// Although our shaders are only GLSL1.10, there are frequent problems with systems just at this level of programmable shaders.
	// If GLSL version is less than 1.30 or GLSL ES 1.00, Stellarium usually does run properly on Windows or various Linux flavours.
	// Depending on whatever driver/implementation details, Stellarium may crash or show only minor graphical errors.
	// On these systems, we show a warning panel that can be suppressed by a config option which is automatically added on first run.
	// Again, based on a sample size of one, Macs have been reported already to always work in this case.
#ifndef Q_OS_MAC
	QRegExp glslRegExp("^(\\d\\.\\d\\d)");
	int pos=glslRegExp.indexIn(glslString);
	// VC4 drivers on Raspberry Pi reports ES 1.0.16 or so, we must step down to one cipher after decimal.
	QRegExp glslesRegExp("ES (\\d\\.\\d)");
	int posES=glslesRegExp.indexIn(glslString);
	if (pos >-1)
	{
		float glslVersion=glslRegExp.cap(1).toFloat();
		qDebug() << "GLSL Version Number detected: " << glslVersion;
		if (glslVersion<1.3f)
		{
			openGLerror=true;
			qDebug() << "This is not enough: we need GLSL1.30 or later.";
			qDebug() << "You should update graphics drivers, graphics hardware, or use the --mesa-mode option.";
			qDebug() << "Else, please try to use an older version like 0.12.5, and try there with --safe-mode";

			if (conf->value("main/ignore_opengl_warning", false).toBool())
			{
				qDebug() << "Config option main/ignore_opengl_warning found, continuing. Expect problems.";
			}
			else
			{
				qDebug() << "You can try to run in an unsupported degraded mode by ignoring the warning and continuing.";
				qDebug() << "But more than likely problems will persist.";
				QMessageBox::StandardButton answerButton=
				QMessageBox::critical(0, "Stellarium", q_("Your OpenGL subsystem has problems. See log for details.\nIgnore and suppress this notice in the future and try to continue in degraded mode anyway?"),
						      QMessageBox::Ignore|QMessageBox::Abort, QMessageBox::Abort);
				if (answerButton == QMessageBox::Abort)
				{
					qDebug() << "Aborting due to OpenGL/GLSL version problems.";
					exit(1);
				}
				else
				{
					qDebug() << "Ignoring all warnings, continuing without further question.";
					conf->setValue("main/ignore_opengl_warning", true);
				}
			}
		}
		else
			qDebug() << "GLSL version is fine, we should not see a graphics problem.";
	}
	else if (posES >-1)
	{
		float glslesVersion=glslesRegExp.cap(1).toFloat();
		qDebug() << "GLSL ES Version Number detected: " << glslesVersion;
		if (glslesVersion<1.0) // TBD: is this possible at all?
		{
			openGLerror=true;
			qDebug() << "This is not enough: we need GLSL ES 1.00 or later.";
#ifdef Q_OS_WIN
			qDebug() << "You should update graphics drivers, graphics hardware, or use the --mesa-mode option.";
#else
			qDebug() << "You should update graphics drivers or graphics hardware.";
#endif
			qDebug() << "Else, please try to use an older version like 0.12.5, and try there with --safe-mode";

			if (conf->value("main/ignore_opengl_warning", false).toBool())
			{
				qDebug() << "Config option main/ignore_opengl_warning found, continuing. Expect problems.";
			}
			else
			{
				qDebug() << "You can try to run in an unsupported degraded mode by ignoring the warning and continuing.";
				qDebug() << "But more than likely problems will persist.";
				QMessageBox::StandardButton answerButton=
				QMessageBox::critical(0, "Stellarium", q_("Your OpenGL ES subsystem has problems. See log for details.\nIgnore and suppress this notice in the future and try to continue in degraded mode anyway?"),
						      QMessageBox::Ignore|QMessageBox::Abort, QMessageBox::Abort);
				if (answerButton == QMessageBox::Abort)
				{
					qDebug() << "Aborting due to OpenGL ES/GLSL ES version problems.";
					exit(1);
				}
				else
				{
					qDebug() << "Ignoring all warnings, continuing without further question.";
					conf->setValue("main/ignore_opengl_warning", true);
				}
			}
		}
		else
		{
			if (openGLerror)
				qDebug() << "GLSL ES version is OK, but there were previous errors, expect problems.";
			else
				qDebug() << "GLSL ES version is fine, we should not see a graphics problem.";
		}
	}
	else
	{
		qDebug() << "Cannot parse GLSL (ES) version string. This may indicate future problems.";
		qDebug() << "Please send a bug report that includes this log file and states if Stellarium works or has problems.";
	}
#endif
}

// Debug info about OpenGL capabilities.
void StelMainView::dumpOpenGLdiagnostics() const
{
	QOpenGLContext *context = QOpenGLContext::currentContext();
	if (context)
	{
		context->functions()->initializeOpenGLFunctions();
		qDebug() << "initializeOpenGLFunctions()...";
		QOpenGLFunctions::OpenGLFeatures oglFeatures=context->functions()->openGLFeatures();
		qDebug() << "OpenGL Features:";
		qDebug() << " - glActiveTexture() function" << (oglFeatures&QOpenGLFunctions::Multitexture ? "is" : "is NOT") << "available.";
		qDebug() << " - Shader functions" << (oglFeatures&QOpenGLFunctions::Shaders ? "are" : "are NOT ") << "available.";
		qDebug() << " - Vertex and index buffer functions" << (oglFeatures&QOpenGLFunctions::Buffers ? "are" : "are NOT") << "available.";
		qDebug() << " - Framebuffer object functions" << (oglFeatures&QOpenGLFunctions::Framebuffers ? "are" : "are NOT") << "available.";
		qDebug() << " - glBlendColor()" << (oglFeatures&QOpenGLFunctions::BlendColor ? "is" : "is NOT") << "available.";
		qDebug() << " - glBlendEquation()" << (oglFeatures&QOpenGLFunctions::BlendEquation ? "is" : "is NOT") << "available.";
		qDebug() << " - glBlendEquationSeparate()" << (oglFeatures&QOpenGLFunctions::BlendEquationSeparate ? "is" : "is NOT") << "available.";
		qDebug() << " - glBlendFuncSeparate()" << (oglFeatures&QOpenGLFunctions::BlendFuncSeparate ? "is" : "is NOT") << "available.";
		qDebug() << " - Blend subtract mode" << (oglFeatures&QOpenGLFunctions::BlendSubtract ? "is" : "is NOT") << "available.";
		qDebug() << " - Compressed texture functions" << (oglFeatures&QOpenGLFunctions::CompressedTextures ? "are" : "are NOT") << "available.";
		qDebug() << " - glSampleCoverage() function" << (oglFeatures&QOpenGLFunctions::Multisample ? "is" : "is NOT") << "available.";
		qDebug() << " - Separate stencil functions" << (oglFeatures&QOpenGLFunctions::StencilSeparate ? "are" : "are NOT") << "available.";
		qDebug() << " - Non power of two textures" << (oglFeatures&QOpenGLFunctions::NPOTTextures ? "are" : "are NOT") << "available.";
		qDebug() << " - Non power of two textures" << (oglFeatures&QOpenGLFunctions::NPOTTextureRepeat ? "can" : "CANNOT") << "use GL_REPEAT as wrap parameter.";
		qDebug() << " - The fixed function pipeline" << (oglFeatures&QOpenGLFunctions::FixedFunctionPipeline ? "is" : "is NOT") << "available.";
		
		qDebug() << "OpenGL shader capabilities and details:";
		qDebug() << " - Vertex Shader:" << (QOpenGLShader::hasOpenGLShaders(QOpenGLShader::Vertex, context) ? "YES" : "NO");
		qDebug() << " - Fragment Shader:" << (QOpenGLShader::hasOpenGLShaders(QOpenGLShader::Fragment, context) ? "YES" : "NO");
		qDebug() << " - Geometry Shader:" << (QOpenGLShader::hasOpenGLShaders(QOpenGLShader::Geometry, context) ? "YES" : "NO");
		qDebug() << " - TessellationControl Shader:" << (QOpenGLShader::hasOpenGLShaders(QOpenGLShader::TessellationControl, context) ? "YES" : "NO");
		qDebug() << " - TessellationEvaluation Shader:" << (QOpenGLShader::hasOpenGLShaders(QOpenGLShader::TessellationEvaluation, context) ? "YES" : "NO");
		qDebug() << " - Compute Shader:" << (QOpenGLShader::hasOpenGLShaders(QOpenGLShader::Compute, context) ? "YES" : "NO");
		
		// GZ: List available extensions. Not sure if this is in any way useful?
		QSet<QByteArray> extensionSet=context->extensions();
		qDebug() << "We have" << extensionSet.count() << "OpenGL extensions:";
		QMap<QString, QString> extensionMap;
		QSetIterator<QByteArray> iter(extensionSet);
		while (iter.hasNext())
		{
			if (!iter.peekNext().isEmpty()) {// Don't insert empty lines
				extensionMap.insert(QString(iter.peekNext()), QString(iter.peekNext()));
			}
			iter.next();
		}
		QMapIterator<QString, QString> iter2(extensionMap);
		while (iter2.hasNext()) {
			qDebug() << " -" << iter2.next().key();
		}
		// Apparently EXT_gpu_shader4 is required for GLSL1.3. (http://en.wikipedia.org/wiki/OpenGL#OpenGL_3.0).
		qDebug() << "EXT_gpu_shader4" << (extensionSet.contains(("EXT_gpu_shader4")) ? "present, OK." : "MISSING!");
		
		QFunctionPointer programParameterPtr =context->getProcAddress("glProgramParameteri");
		if (programParameterPtr == 0) {
			qDebug() << "glProgramParameteri cannot be resolved here. BAD!";
		}
		programParameterPtr =context->getProcAddress("glProgramParameteriEXT");
		if (programParameterPtr == 0) {
			qDebug() << "glProgramParameteriEXT cannot be resolved here. BAD!";
		}
	}
	else
	{
		qDebug() << "dumpOpenGLdiagnostics(): No OpenGL context";
	}


}


void StelMainView::deinit()
{
	glContextMakeCurrent();
	deinitGL();
	delete stelApp;
	stelApp = NULL;
}

// Update the translated title
void StelMainView::initTitleI18n()
{
	QString appNameI18n = q_("Stellarium %1").arg(StelUtils::getApplicationVersion());
	setWindowTitle(appNameI18n);
}

void StelMainView::setFullScreen(bool b)
{
	if (b)
		showFullScreen();
	else
	{
		showNormal();
		// Not enough. If we had started in fullscreen, the inner part of the window is at 0/0, with the frame extending to top/left off screen.
		// Therefore moving is not possible. We must move to the stored position or at least defaults.
		if ( (x()<0)  && (y()<0))
		{
			QSettings *conf = stelApp->getSettings();
			QDesktopWidget *desktop = QApplication::desktop();
			int screen = conf->value("video/screen_number", 0).toInt();
			if (screen < 0 || screen >= desktop->screenCount())
			{
				qWarning() << "WARNING: screen" << screen << "not found";
				screen = 0;
			}
			QRect screenGeom = desktop->screenGeometry(screen);
			int x = conf->value("video/screen_x", 0).toInt();
			int y = conf->value("video/screen_y", 0).toInt();
			move(x + screenGeom.x(), y + screenGeom.y());
		}
	}
	emit fullScreenChanged(b);
}

void StelMainView::drawEnded()
{
	updateQueued = false;

	//requeue the next draw
	if(needsMaxFPS())
	{
		updateQueued = true;
		minFpsTimer->stop();
		glWidget->update();
	}
	else
	{
		if(!minFpsTimer->isActive())
			minFpsTimer->start();
	}
}

void StelMainView::minFPSUpdate()
{
	if(!updateQueued)
	{
		updateQueued = true;
		//qDebug()<<"minFPSUpdate";
		glWidget->update();
	}
	else
	{
		//qDebug()<<"double update";
	}
}

#ifdef OPENGL_DEBUG_LOGGING
void StelMainView::logGLMessage(const QOpenGLDebugMessage &debugMessage)
{
	qDebug()<<debugMessage;
}

void StelMainView::contextDestroyed()
{
	qDebug()<<"Main OpenGL context destroyed";
}
#endif

void StelMainView::thereWasAnEvent()
{
	//qDebug()<<"event";
	lastEventTimeSec = StelApp::getTotalRunTime();
}

bool StelMainView::needsMaxFPS() const
{
	const double now = StelApp::getTotalRunTime();

	// Determines when the next display will need to be triggered
	// The current policy is that after an event, the FPS is maximum for 2.5 seconds
	// after that, it switches back to the default minfps value to save power.
	// The fps is also kept to max if the timerate is higher than normal speed.
	const float timeRate = stelApp->getCore()->getTimeRate();
	return (now - lastEventTimeSec < 2.5) || fabs(timeRate) > StelCore::JD_SECOND;
}

void StelMainView::moveEvent(QMoveEvent * event)
{
	Q_UNUSED(event);

	// We use the glWidget instead of the event, as we want the screen that shows most of the widget.
	QWindow* win = glWidget->windowHandle();
	if(win)
		stelApp->setDevicePixelsPerPixel(win->devicePixelRatio());
}

void StelMainView::closeEvent(QCloseEvent* event)
{
	Q_UNUSED(event);
	stelApp->quit();
}

//! Delete openGL textures (to call before the GLContext disappears)
void StelMainView::deinitGL()
{
	//fix for bug 1628072 caused by QTBUG-56798
#ifndef QT_NO_DEBUG
	StelOpenGL::clearGLErrors();
#endif

	stelApp->deinit();
	delete gui;
	gui = NULL;
}

void StelMainView::saveScreenShot(const QString& filePrefix, const QString& saveDir, const bool overwrite)
{
	screenShotPrefix = filePrefix;
	screenShotDir = saveDir;
	flagOverwriteScreenshots=overwrite;
	emit(screenshotRequested());
}

void StelMainView::doScreenshot(void)
{
	QFileInfo shotDir;
#ifdef USE_OLD_QGLWIDGET
	QImage im = glWidget->grabFrameBuffer();
#else
	glWidget->makeCurrent();
	QOpenGLFramebufferObjectFormat fbFormat;
	fbFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
	QOpenGLFramebufferObject * fbObj = new QOpenGLFramebufferObject(stelScene->width(), stelScene->height(), fbFormat);
	fbObj->bind();
	QOpenGLPaintDevice fbObjPaintDev(stelScene->width(), stelScene->height());
	QPainter painter(&fbObjPaintDev);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
	stelScene->render(&painter);
	painter.end();
	QImage im = fbObj->toImage();
	fbObj->release();
	delete fbObj;
#endif

	if (flagInvertScreenShotColors)
		im.invertPixels();

	if (StelFileMgr::getScreenshotDir().isEmpty())
	{
		qWarning() << "Oops, the directory for screenshots is not set! Let's try create and set it...";
		// Create a directory for screenshots if main/screenshot_dir option is unset and user do screenshot at the moment!
		QString screenshotDirSuffix = "/Stellarium";
		QString screenshotDir;
		if (!QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).isEmpty())
			screenshotDir = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation)[0].append(screenshotDirSuffix);
		else
			screenshotDir = StelFileMgr::getUserDir().append(screenshotDirSuffix);

		try
		{
			StelFileMgr::setScreenshotDir(screenshotDir);
			StelApp::getInstance().getSettings()->setValue("main/screenshot_dir", screenshotDir);
		}
		catch (std::runtime_error &e)
		{
			qDebug("Error: cannot create screenshot directory: %s", e.what());
		}
	}

	if (screenShotDir == "")
		shotDir = QFileInfo(StelFileMgr::getScreenshotDir());
	else
		shotDir = QFileInfo(screenShotDir);

	if (!shotDir.isDir())
	{
		qWarning() << "ERROR requested screenshot directory is not a directory: " << QDir::toNativeSeparators(shotDir.filePath());
		return;
	}
	else if (!shotDir.isWritable())
	{
		qWarning() << "ERROR requested screenshot directory is not writable: " << QDir::toNativeSeparators(shotDir.filePath());
		return;
	}

	QFileInfo shotPath;
	if (flagOverwriteScreenshots)
	{
		shotPath = QFileInfo(shotDir.filePath() + "/" + screenShotPrefix + ".png");
	}
	else
	{
		for (int j=0; j<100000; ++j)
		{
			shotPath = QFileInfo(shotDir.filePath() + "/" + screenShotPrefix + QString("%1").arg(j, 3, 10, QLatin1Char('0')) + ".png");
			if (!shotPath.exists())
				break;
		}
	}
	qDebug() << "INFO Saving screenshot in file: " << QDir::toNativeSeparators(shotPath.filePath());
	if (!im.save(shotPath.filePath())) {
		qWarning() << "WARNING failed to write screenshot to: " << QDir::toNativeSeparators(shotPath.filePath());
	}
}

QPoint StelMainView::getMousePos()
{
	return glWidget->mapFromGlobal(QCursor::pos());
}

QOpenGLContext* StelMainView::glContext() const
{
#ifdef USE_OLD_QGLWIDGET
	return glWidget->context()->contextHandle();
#else
	return glWidget->context();
#endif
}

void StelMainView::glContextMakeCurrent()
{
	glWidget->makeCurrent();
}

void StelMainView::glContextDoneCurrent()
{
	glWidget->doneCurrent();
}
