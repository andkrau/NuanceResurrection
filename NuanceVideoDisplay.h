#ifndef NUANCEVIDEODISPLAY_H
#define NUANCEVIDEODISPLAY_H

#include <qdialog.h>
#include <qobject.h>
#include "mpe.h"
#include "NuanceGLWidget.h"
#include "video.h"

class NuanceVideoDisplay : public QDialog
{
  Q_OBJECT

public:
  NuanceVideoDisplay(QGLFormat &format, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0);
  NuanceVideoDisplay(QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0);
 ~NuanceVideoDisplay();
  void UpdateDisplay();

public slots:

protected:
  void keyPressEvent(QKeyEvent *);
  void keyReleaseEvent(QKeyEvent *);
  void resizeEvent(QResizeEvent *);
  void paintEvent (QPaintEvent *);
private:
  NuanceGLWidget *glWidget;
  virtual void timerEvent(QTimerEvent *timerEvent);
  bool bGlewInitialized;
};
#endif
