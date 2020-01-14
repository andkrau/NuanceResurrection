#include "glew.h"
#include "NuonEnvironment.h"
#include "NuanceGLWidget.h"
#include "video.h"

extern NuonEnvironment *nuonEnv;
extern bool bProcessorThreadRunning;
extern uint8 *mainChannelBuffer;
extern uint8 *overlayChannelBuffer;

uint8 *AllocateTextureMemory(uint32 size, bool bOverlay);
void FreeTextureMemory(uint8 *ptr, bool bOverlay);

NuanceGLWidget::NuanceGLWidget( QWidget* parent, const char* name, const QGLWidget* shareWidget) : QGLWidget(parent, name, shareWidget)
{
}
NuanceGLWidget::NuanceGLWidget( const QGLFormat& format, QWidget* parent, const char* name, const QGLWidget* shareWidget) : QGLWidget(format, parent, name, shareWidget)
{
}

NuanceGLWidget::~NuanceGLWidget()
{
  FreeTextureMemory(mainChannelBuffer,false);
  FreeTextureMemory(overlayChannelBuffer,true);
}

void NuanceGLWidget::initializeGL()
{
  mainChannelBuffer = AllocateTextureMemory(ALLOCATED_TEXTURE_WIDTH*ALLOCATED_TEXTURE_HEIGHT*4,false);
  overlayChannelBuffer = AllocateTextureMemory(ALLOCATED_TEXTURE_WIDTH*ALLOCATED_TEXTURE_HEIGHT*4,true);
  InitializeYCrCbColorSpace();
}

void NuanceGLWidget::resizeGL(int w, int h)
{
  glViewport(0,0,w,h);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0,1.0,0.0,1.0,-1.0,1.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_ALPHA_TEST);
  glDisable(GL_LIGHTING);
  glDisable(GL_FOG);

  glClear(GL_COLOR_BUFFER_BIT);
  nuonEnv->bMainBufferModified = true;
  nuonEnv->bOverlayBufferModified = true;
}


void NuanceGLWidget::paintGL()
{
  if(bProcessorThreadRunning)
  {
    //nuonEnv->bMainBufferModified = true;
    //nuonEnv->bOverlayBufferModified = true;
    RenderVideo(width(),height());
  }
  else
  {
    glClear(GL_COLOR_BUFFER_BIT);
  }
}
