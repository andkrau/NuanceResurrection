#include "glew.h"
#include "Byteswap.h"
#include "NuanceVideoDisplay.h"
#include "NuanceGLWidget.h"
#include "NuonEnvironment.h"
#include "QPixmap.h"
#include "joystick.h"
#include "mpe.h"
#include "qgl.h"
#include "video.h"

extern NuonEnvironment *nuonEnv;
extern ControllerData *controller;
extern MPE *videoRequestSource;
extern volatile eVideoRequest currentVideoRequest;

NuanceVideoDisplay::NuanceVideoDisplay(QWidget* parent, const char* name, bool modal, WFlags fl) : QDialog( parent, name, modal, fl )
{
  if(!name)
  {
	  setName( "NuanceVideoDisplay" );
  }
  setIcon(QPixmap("nuance.bmp","BMP"));
  setCaption( tr( "Nuance Video Display" ) );
  resize(720, 480); 
  bGlewInitialized = false;
  glWidget = new NuanceGLWidget(this,"NuanceGLWidget");
  startTimer(0);
}

NuanceVideoDisplay::NuanceVideoDisplay(QGLFormat &format, QWidget* parent, const char* name, bool modal, WFlags fl) : QDialog( parent, name, modal, fl )
{
  if(!name)
  {
	  setName("NuanceVideoDisplay");
  }
  setIcon(QPixmap("nuance.bmp","BMP"));
  resize( 720, 480 ); 
  setCaption( tr( "Nuance Video Display" ) );

  bGlewInitialized = false;
  glWidget = new NuanceGLWidget(format,this,"NuanceGLWidget");
  startTimer(0);
}

NuanceVideoDisplay::~NuanceVideoDisplay()
{
  killTimers();
  delete glWidget;
}

void NuanceVideoDisplay::resizeEvent(QResizeEvent *event)
{
  QSize size = event->size();
  glWidget->resize(size.width(),size.height());
}

void NuanceVideoDisplay::paintEvent(QPaintEvent *event)
{
  nuonEnv->bMainBufferModified = true;
  nuonEnv->bOverlayBufferModified = true;
  UpdateDisplay();
}

void NuanceVideoDisplay::keyPressEvent(QKeyEvent *keyEvent)
{
  uint16 buttons = 0;

  switch(keyEvent->key())
  {
    case Key_A:
      //Start
      buttons = CTRLR_BUTTON_START;
      break;
    case Key_S:
      //NUON (Z)
      buttons = CTRLR_BUTTON_NUON;
      break;
    case Key_D:
      //A
      buttons = CTRLR_BUTTON_A;
      break;
    case Key_F:
      //B
      buttons = CTRLR_BUTTON_B;
      break;
    case Key_Up:
      //DPad Up
      buttons = CTRLR_DPAD_UP;
      break;
    case Key_Down:
      //DPad Down
      buttons = CTRLR_DPAD_DOWN;
      break;
    case Key_Left:
      //DPad Left
      buttons = CTRLR_DPAD_LEFT;
      break;
    case Key_Right:
      //DPad Right
      buttons = CTRLR_DPAD_RIGHT;
      break;
    case Key_Q:
      //LShoulder
      buttons = CTRLR_BUTTON_L;
      break;
    case Key_T:
      //RShoulder
      buttons = CTRLR_BUTTON_R;
      break;
    case Key_W:
      //C left
      buttons = CTRLR_BUTTON_C_LEFT;
      break;
    case Key_E:
      //C down
      buttons = CTRLR_BUTTON_C_DOWN;
      break;
    case Key_R:
      //C right
      buttons = CTRLR_BUTTON_C_RIGHT;
      break;
    case Key_3:
      //C up
      buttons = CTRLR_BUTTON_C_UP;
      break;
  }
  SwapWordBytes(&buttons);
  if(controller)
  {
    controller[1].buttons |= buttons;
  }
}

void NuanceVideoDisplay::keyReleaseEvent(QKeyEvent *keyEvent)
{
  uint16 buttons = 0;

  switch(keyEvent->key())
  {
    case Key_A:
      //Start
      buttons = ~CTRLR_BUTTON_START;
      break;
    case Key_S:
      //NUON (Z)
      buttons = ~CTRLR_BUTTON_NUON;
      break;
    case Key_D:
      //A
      buttons = ~CTRLR_BUTTON_A;
      break;
    case Key_F:
      //B
      buttons = ~CTRLR_BUTTON_B;
      break;
    case Key_Up:
      //DPad Up
      buttons = ~CTRLR_DPAD_UP;
      break;
    case Key_Down:
      //DPad Down
      buttons = ~CTRLR_DPAD_DOWN;
      break;
    case Key_Left:
      //DPad Left
      buttons = ~CTRLR_DPAD_LEFT;
      break;
    case Key_Right:
      //DPad Right
      buttons = ~CTRLR_DPAD_RIGHT;
      break;
    case Key_Q:
      //LShoulder
      buttons = ~CTRLR_BUTTON_L;
      break;
    case Key_T:
      //RShoulder
      buttons = ~CTRLR_BUTTON_R;
      break;
    case Key_W:
      //C left
      buttons = ~CTRLR_BUTTON_C_LEFT;
      break;
    case Key_E:
      //C down
      buttons = ~CTRLR_BUTTON_C_DOWN;
      break;
    case Key_R:
      //C right
      buttons = ~CTRLR_BUTTON_C_RIGHT;
      break;
    case Key_3:
      //C up
      buttons = ~CTRLR_BUTTON_C_UP;
      break;
  }
  SwapWordBytes(&buttons);
  if(controller)
  {
    controller[1].buttons &= buttons;
  }
}

void NuanceVideoDisplay::UpdateDisplay(void)
{
  glWidget->UpdateDisplay();
}

void NuanceVideoDisplay::timerEvent(QTimerEvent *timerEvent)
{
  if(videoRequestSource && (currentVideoRequest != VIDEOREQUEST_NONE))
  {
    switch(currentVideoRequest)
    {
      case VIDEOREQUEST_INITGLEW:
        break;
      case VIDEOREQUEST_VIDCONFIG:
        VidConfig(videoRequestSource);
        break;
      case VIDEOREQUEST_VIDSETUP:
        VidSetup(videoRequestSource);
        break;
      case VIDEOREQUEST_SETDEFAULTCOLOR:
        SetDefaultColor(videoRequestSource);
        break;
      case VIDEOREQUEST_SETVIDEOMODE:
        SetVideoMode();
        break;
      case VIDEOREQUEST_VIDCHANGEBASE:
        VidChangeBase(videoRequestSource);
        break;
      case VIDEOREQUEST_VIDCHANGESCROLL:
        VidChangeScroll(videoRequestSource);
        break;
      case VIDEOREQUEST_VIDSETCLUTRANGE:
        VidSetCLUTRange(videoRequestSource);
        break;
    }
  }
  currentVideoRequest = VIDEOREQUEST_NONE;
}