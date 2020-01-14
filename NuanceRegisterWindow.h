#ifndef NUANCEREGISTERWINDOW_H
#define NUANCEREGISTERWINDOW_H

#include "Basetypes.h"
#include "RegisterWindow.h"
#include "QApplication.h"
#include "QPixmap.h"
#include "QTextView.h"

class NuanceRegisterWindow : public RegisterWindow
{ 
  Q_OBJECT

public:
  void RegisterApplication(QApplication *app) {registeredApp = app;}
  NuanceRegisterWindow( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 ) : RegisterWindow(parent,name,modal,fl)
  {
    setIcon(QPixmap("nuance.bmp","BMP"));
    this->setMinimumWidth(799);
    this->setMinimumHeight(514);
    this->setMaximumWidth(799);
    //this->setMaximumHeight(514);
    currOffset = 0;
  }
  void UpdateDebugDisplay(uint32 whichMPE);
  void SetCurrentMPE(uint32 which) { currMPE = which; }
public slots:
    virtual void OnMemoryBankSelect(int);
    virtual void OnMemoryPageChange(int);
protected:
  uint32 currOffset;
  uint32 currMPE;
  QApplication *registeredApp;
};

#endif
