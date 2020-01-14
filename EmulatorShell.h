/****************************************************************************
** Form interface generated from reading ui file 'EmulatorShell.ui'
**
** Created: Fri Sep 26 22:51:32 2003
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef EMULATORSHELL_H
#define EMULATORSHELL_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QComboBox;
class QLabel;
class QMultiLineEdit;
class QPushButton;

class EmulatorShell : public QDialog
{ 
    Q_OBJECT

public:
    EmulatorShell( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~EmulatorShell();

    QLabel* TextLabel3_2;
    QLabel* TextLabel3;
    QLabel* TextLabel2;
    QLabel* TextLabel2_2;
    QMultiLineEdit* ebStdOut;
    QMultiLineEdit* ebStdErr;
    QPushButton* cbLoadFile;
    QPushButton* cbSingleStepMPE;
    QPushButton* cbStartMPE;
    QPushButton* cbStopMPE;
    QPushButton* cbResetMPE;
    QPushButton* cbRunMPE;
    QComboBox* ddlbMPESelect;
    QComboBox* ddlbRefreshRate;

protected slots:
    virtual void OnMPESelect(int);
    virtual void OnRefreshRate(int);
    virtual void OnLoadFile();
    virtual void OnRunMPE();
    virtual void OnStopMPE();
    virtual void OnSingleStepMPE();
    virtual void OnStartMPE();
    virtual void OnResetMPE();

};

#endif // EMULATORSHELL_H
