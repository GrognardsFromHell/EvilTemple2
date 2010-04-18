#include "dialog.h"
#include "ui_dialog.h"

#include <QGLWidget>

#if defined Q_OS_WIN32
#include <windows.h>
#include "wglext.h"

bool WGLExtensionSupported(const char *extension_name)
{
    // this is pointer to function which returns pointer to string with list of all wgl extensions
    PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglGetExtensionsStringEXT = NULL;

    // determine pointer to wglGetExtensionsStringEXT function
    _wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC) wglGetProcAddress("wglGetExtensionsStringEXT");

    if (strstr(_wglGetExtensionsStringEXT(), extension_name) == NULL)
    {
        // string was not found
        return false;
    }

    // extension is supported
    return true;
}

PFNWGLSWAPINTERVALEXTPROC       wglSwapIntervalEXT = NULL;
PFNWGLGETSWAPINTERVALEXTPROC    wglGetSwapIntervalEXT = NULL;

#endif

class MyGlWidget : public QGLWidget
{
public:
    MyGlWidget(QWidget *parent) : QGLWidget(parent) {
    }

    QImage movieFrame;


protected:
    void initializeGL()
    {
#ifdef Q_OS_WIN32
        if (WGLExtensionSupported("WGL_EXT_swap_control")) {
            // Extension is supported, init pointers.
            wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC) wglGetProcAddress("wglSwapIntervalEXT");

            // this is another function from WGL_EXT_swap_control extension
            wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC) wglGetProcAddress("wglGetSwapIntervalEXT");

            wglSwapIntervalEXT(1);
        }
#endif
    }

    void resizeGL(int width, int height)
    {
        glViewport(0, 0, width, height);
        glLoadIdentity();
        gluOrtho2D(0, 1, 1, 0);
    }

    void paintGL()
    {
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        glEnable(GL_TEXTURE_2D);
        GLuint texture = bindTexture(movieFrame, GL_TEXTURE_2D, GL_RGBA, QGLContext::NoBindOption);

        glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex2i(0, 0);
        glTexCoord2f(0, 1);
        glVertex2i(0, 1);
        glTexCoord2f(1, 1);
        glVertex2i(1, 1);
        glTexCoord2f(1, 0);
        glVertex2i(1, 0);
        glEnd();

        deleteTexture(texture);
    }
};

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    glWidget = new MyGlWidget(this);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::resizeEvent(QResizeEvent * event)
{
    QDialog::resizeEvent(event);

    glWidget->resize(size());
}

void Dialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void Dialog::showFrame(const QImage &frame)
{
    glWidget->movieFrame = frame;
    glWidget->update();
}
