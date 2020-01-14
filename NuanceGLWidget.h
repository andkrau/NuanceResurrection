#ifndef NUANCEGLWIDGET_H
#define NUANCEGLWIDGET_H

#include <qgl.h>

class NuanceGLWidget : public QGLWidget
{
  Q_OBJECT

public:

  NuanceGLWidget( QWidget* parent, const char* name, const QGLWidget* shareWidget=0 );
  NuanceGLWidget( const QGLFormat& format, QWidget* parent, const char* name, const QGLWidget* shareWidget=0 );
  ~NuanceGLWidget();
  void UpdateDisplay() { paintGL(); }

public slots:

protected:
  void initializeGL();
  void paintGL();
  void resizeGL( int w, int h );
private:

};
#endif
