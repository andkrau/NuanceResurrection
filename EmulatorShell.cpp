/****************************************************************************
** Form implementation generated from reading ui file 'EmulatorShell.ui'
**
** Created: Fri Sep 26 22:52:11 2003
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "EmulatorShell.h"

#include <qcombobox.h>
#include <qlabel.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a EmulatorShell which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
EmulatorShell::EmulatorShell( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "EmulatorShell" );
    resize( 669, 301 ); 
    setCaption( tr( "Nuance Emulator Shell" ) );

    TextLabel3_2 = new QLabel( this, "TextLabel3_2" );
    TextLabel3_2->setGeometry( QRect( 550, 200, 80, 16 ) ); 
    TextLabel3_2->setText( tr( "Refresh Rate" ) );

    TextLabel3 = new QLabel( this, "TextLabel3" );
    TextLabel3->setGeometry( QRect( 430, 200, 80, 16 ) ); 
    TextLabel3->setText( tr( "MPE Select" ) );

    TextLabel2 = new QLabel( this, "TextLabel2" );
    TextLabel2->setGeometry( QRect( 90, 260, 60, 21 ) ); 
    TextLabel2->setText( tr( "STDOOUT" ) );

    TextLabel2_2 = new QLabel( this, "TextLabel2_2" );
    TextLabel2_2->setGeometry( QRect( 290, 260, 60, 21 ) ); 
    TextLabel2_2->setText( tr( "STDERR" ) );

    ebStdOut = new QMultiLineEdit( this, "ebStdOut" );
    ebStdOut->setGeometry( QRect( 10, 10, 200, 240 ) ); 
    ebStdOut->setReadOnly( TRUE );

    ebStdErr = new QMultiLineEdit( this, "ebStdErr" );
    ebStdErr->setGeometry( QRect( 220, 10, 200, 241 ) ); 
    ebStdErr->setReadOnly( TRUE );

    cbLoadFile = new QPushButton( this, "cbLoadFile" );
    cbLoadFile->setGeometry( QRect( 430, 10, 111, 51 ) ); 
    cbLoadFile->setText( tr( "Load File" ) );

    cbSingleStepMPE = new QPushButton( this, "cbSingleStepMPE" );
    cbSingleStepMPE->setGeometry( QRect( 550, 130, 111, 51 ) ); 
    cbSingleStepMPE->setText( tr( "Single Step" ) );

    cbStartMPE = new QPushButton( this, "cbStartMPE" );
    cbStartMPE->setGeometry( QRect( 430, 130, 111, 51 ) ); 
    cbStartMPE->setText( tr( "Start MPE" ) );

    cbStopMPE = new QPushButton( this, "cbStopMPE" );
    cbStopMPE->setGeometry( QRect( 550, 70, 111, 51 ) ); 
    cbStopMPE->setText( tr( "Stop" ) );

    cbResetMPE = new QPushButton( this, "cbResetMPE" );
    cbResetMPE->setGeometry( QRect( 430, 70, 111, 51 ) ); 
    cbResetMPE->setText( tr( "Reset MPE" ) );

    cbRunMPE = new QPushButton( this, "cbRunMPE" );
    cbRunMPE->setGeometry( QRect( 550, 10, 111, 51 ) ); 
    cbRunMPE->setText( tr( "Run" ) );

    ddlbMPESelect = new QComboBox( FALSE, this, "ddlbMPESelect" );
    ddlbMPESelect->insertItem( tr( "MPE0" ) );
    ddlbMPESelect->insertItem( tr( "MPE1" ) );
    ddlbMPESelect->insertItem( tr( "MPE2" ) );
    ddlbMPESelect->insertItem( tr( "MPE3" ) );
    ddlbMPESelect->setGeometry( QRect( 430, 220, 80, 20 ) ); 

    ddlbRefreshRate = new QComboBox( FALSE, this, "ddlbRefreshRate" );
    ddlbRefreshRate->insertItem( tr( "15 Hz" ) );
    ddlbRefreshRate->insertItem( tr( "30 Hz" ) );
    ddlbRefreshRate->insertItem( tr( "45 Hz" ) );
    ddlbRefreshRate->insertItem( tr( "60 Hz" ) );
    ddlbRefreshRate->setGeometry( QRect( 550, 220, 80, 20 ) ); 

    // signals and slots connections
    connect( cbLoadFile, SIGNAL( clicked() ), this, SLOT( OnLoadFile() ) );
    connect( cbRunMPE, SIGNAL( clicked() ), this, SLOT( OnRunMPE() ) );
    connect( cbResetMPE, SIGNAL( clicked() ), this, SLOT( OnResetMPE() ) );
    connect( cbStopMPE, SIGNAL( clicked() ), this, SLOT( OnStopMPE() ) );
    connect( cbStartMPE, SIGNAL( clicked() ), this, SLOT( OnStartMPE() ) );
    connect( cbSingleStepMPE, SIGNAL( clicked() ), this, SLOT( OnSingleStepMPE() ) );
    connect( ddlbMPESelect, SIGNAL( activated(int) ), this, SLOT( OnMPESelect(int) ) );
    connect( ddlbRefreshRate, SIGNAL( activated(int) ), this, SLOT( OnRefreshRate(int) ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
EmulatorShell::~EmulatorShell()
{
    // no need to delete child widgets, Qt does it all for us
}

void EmulatorShell::OnMPESelect(int)
{
    qWarning( "EmulatorShell::OnMPESelect(int): Not implemented yet!" );
}

void EmulatorShell::OnRefreshRate(int)
{
    qWarning( "EmulatorShell::OnRefreshRate(int): Not implemented yet!" );
}

void EmulatorShell::OnLoadFile()
{
    qWarning( "EmulatorShell::OnLoadFile(): Not implemented yet!" );
}

void EmulatorShell::OnRunMPE()
{
    qWarning( "EmulatorShell::OnRunMPE(): Not implemented yet!" );
}

void EmulatorShell::OnStopMPE()
{
    qWarning( "EmulatorShell::OnStopMPE(): Not implemented yet!" );
}

void EmulatorShell::OnSingleStepMPE()
{
    qWarning( "EmulatorShell::OnSingleStepMPE(): Not implemented yet!" );
}

void EmulatorShell::OnStartMPE()
{
    qWarning( "EmulatorShell::OnStartMPE(): Not implemented yet!" );
}

void EmulatorShell::OnResetMPE()
{
    qWarning( "EmulatorShell::OnResetMPE(): Not implemented yet!" );
}

