/****************************************************************************
** Form implementation generated from reading ui file 'RegisterWindow.ui'
**
** Created: Sat Oct 25 03:00:10 2003
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "RegisterWindow.h"

#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qtextview.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a RegisterWindow which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
RegisterWindow::RegisterWindow( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "RegisterWindow" );
    resize( 799, 782 ); 
    setMinimumSize( QSize( 799, 514 ) );
    setMaximumSize( QSize( 799, 32767 ) );
    setCaption( tr( "Nuance Register Window" ) );

    tbExcepsrc = new QLineEdit( this, "tbExcepsrc" );
    tbExcepsrc->setGeometry( QRect( 70, 60, 100, 22 ) ); 
    tbExcepsrc->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, tbExcepsrc->sizePolicy().hasHeightForWidth() ) );
    tbExcepsrc->setText( tr( "" ) );

    tbR31 = new QLineEdit( this, "tbR31" );
    tbR31->setGeometry( QRect( 690, 480, 100, 22 ) ); 
    tbR31->setText( tr( "" ) );

    TextLabel1_2_2_2_2_2_4_3_2 = new QLabel( this, "TextLabel1_2_2_2_2_2_4_3_2" );
    TextLabel1_2_2_2_2_2_4_3_2->setGeometry( QRect( 661, 180, 20, 20 ) ); 
    TextLabel1_2_2_2_2_2_4_3_2->setText( tr( "r21" ) );
    TextLabel1_2_2_2_2_2_4_3_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_3_2_3_2 = new QLabel( this, "TextLabel1_3_2_3_2" );
    TextLabel1_3_2_3_2->setGeometry( QRect( 661, 270, 20, 20 ) ); 
    TextLabel1_3_2_3_2->setText( tr( "r24" ) );
    TextLabel1_3_2_3_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbR23 = new QLineEdit( this, "tbR23" );
    tbR23->setGeometry( QRect( 690, 240, 100, 22 ) ); 
    tbR23->setText( tr( "" ) );

    tbR2 = new QLineEdit( this, "tbR2" );
    tbR2->setGeometry( QRect( 550, 90, 100, 22 ) ); 
    tbR2->setText( tr( "" ) );

    tbR14 = new QLineEdit( this, "tbR14" );
    tbR14->setGeometry( QRect( 550, 450, 100, 22 ) ); 
    tbR14->setText( tr( "" ) );

    tbCommxmit0 = new QLineEdit( this, "tbCommxmit0" );
    tbCommxmit0->setGeometry( QRect( 410, 270, 100, 22 ) ); 
    tbCommxmit0->setText( tr( "" ) );

    TextLabel1_2_2_2_3_2 = new QLabel( this, "TextLabel1_2_2_2_3_2" );
    TextLabel1_2_2_2_3_2->setGeometry( QRect( 191, 360, 30, 20 ) ); 
    TextLabel1_2_2_2_3_2->setText( tr( "uvctl" ) );
    TextLabel1_2_2_2_3_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1 = new QLabel( this, "TextLabel1" );
    TextLabel1->setGeometry( QRect( 21, 30, 40, 20 ) ); 
    TextLabel1->setText( tr( "mpectl" ) );
    TextLabel1->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_2_2_2_2_2_4_3 = new QLabel( this, "TextLabel1_2_2_2_2_2_4_3" );
    TextLabel1_2_2_2_2_2_4_3->setGeometry( QRect( 521, 180, 20, 20 ) ); 
    TextLabel1_2_2_2_2_2_4_3->setText( tr( "r5" ) );
    TextLabel1_2_2_2_2_2_4_3->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_2_2_4_2 = new QLabel( this, "TextLabel1_2_2_4_2" );
    TextLabel1_2_2_4_2->setGeometry( QRect( 341, 90, 60, 20 ) ); 
    TextLabel1_2_2_4_2->setText( tr( "odmactl" ) );
    TextLabel1_2_2_4_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_3_2_2_4_2 = new QLabel( this, "TextLabel1_3_2_2_4_2" );
    TextLabel1_3_2_2_4_2->setGeometry( QRect( 340, 480, 60, 20 ) ); 
    TextLabel1_3_2_2_4_2->setText( tr( "commrecv3" ) );
    TextLabel1_3_2_2_4_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_2_2_2_2_4_2 = new QLabel( this, "TextLabel1_2_2_2_2_4_2" );
    TextLabel1_2_2_2_2_4_2->setGeometry( QRect( 361, 150, 40, 20 ) ); 
    TextLabel1_2_2_2_2_4_2->setText( tr( "mdmactl" ) );
    TextLabel1_2_2_2_2_4_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_2_2_4 = new QLabel( this, "TextLabel1_2_2_4" );
    TextLabel1_2_2_4->setGeometry( QRect( 201, 90, 20, 20 ) ); 
    TextLabel1_2_2_4->setText( tr( "rx" ) );
    TextLabel1_2_2_4->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_2_2_2_2_2_4 = new QLabel( this, "TextLabel1_2_2_2_2_2_4" );
    TextLabel1_2_2_2_2_2_4->setGeometry( QRect( 180, 180, 40, 20 ) ); 
    TextLabel1_2_2_2_2_2_4->setText( tr( "xyrange" ) );
    TextLabel1_2_2_2_2_2_4->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbRu = new QLineEdit( this, "tbRu" );
    tbRu->setGeometry( QRect( 230, 240, 100, 22 ) ); 
    tbRu->setText( tr( "" ) );

    TextLabel1_2_3_2 = new QLabel( this, "TextLabel1_2_3_2" );
    TextLabel1_2_3_2->setGeometry( QRect( 181, 300, 40, 20 ) ); 
    TextLabel1_2_3_2->setText( tr( "uvrange" ) );
    TextLabel1_2_3_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_2_2_3_2 = new QLabel( this, "TextLabel1_2_2_3_2" );
    TextLabel1_2_2_3_2->setGeometry( QRect( 181, 330, 40, 20 ) ); 
    TextLabel1_2_2_3_2->setText( tr( "uvbase" ) );
    TextLabel1_2_2_3_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_2_2_2_2_2_2_3_2 = new QLabel( this, "TextLabel1_2_2_2_2_2_2_3_2" );
    TextLabel1_2_2_2_2_2_2_3_2->setGeometry( QRect( 191, 450, 30, 20 ) ); 
    TextLabel1_2_2_2_2_2_2_3_2->setText( tr( "svshift" ) );
    TextLabel1_2_2_2_2_2_2_3_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_2_2_3_2_2 = new QLabel( this, "TextLabel1_2_2_3_2_2" );
    TextLabel1_2_2_3_2_2->setGeometry( QRect( 521, 330, 20, 20 ) ); 
    TextLabel1_2_2_3_2_2->setText( tr( "r10" ) );
    TextLabel1_2_2_3_2_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbR8 = new QLineEdit( this, "tbR8" );
    tbR8->setGeometry( QRect( 550, 270, 100, 22 ) ); 
    tbR8->setText( tr( "" ) );

    TextLabel1_2_2_2_2_2_4_2 = new QLabel( this, "TextLabel1_2_2_2_2_2_4_2" );
    TextLabel1_2_2_2_2_2_4_2->setGeometry( QRect( 351, 180, 50, 20 ) ); 
    TextLabel1_2_2_2_2_2_4_2->setText( tr( "mdmacptr" ) );
    TextLabel1_2_2_2_2_2_4_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbRv = new QLineEdit( this, "tbRv" );
    tbRv->setGeometry( QRect( 230, 270, 100, 22 ) ); 
    tbRv->setText( tr( "" ) );

    tbPcroute = new QLineEdit( this, "tbPcroute" );
    tbPcroute->setGeometry( QRect( 70, 180, 100, 22 ) ); 
    tbPcroute->setText( tr( "" ) );

    TextLabel1_2_2_3_2_2_2 = new QLabel( this, "TextLabel1_2_2_3_2_2_2" );
    TextLabel1_2_2_3_2_2_2->setGeometry( QRect( 660, 330, 20, 20 ) ); 
    TextLabel1_2_2_3_2_2_2->setText( tr( "r26" ) );
    TextLabel1_2_2_3_2_2_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_2_2_4_3_2 = new QLabel( this, "TextLabel1_2_2_4_3_2" );
    TextLabel1_2_2_4_3_2->setGeometry( QRect( 661, 90, 20, 20 ) ); 
    TextLabel1_2_2_4_3_2->setText( tr( "r18" ) );
    TextLabel1_2_2_4_3_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_2_2_2_3_2_2 = new QLabel( this, "TextLabel1_2_2_2_3_2_2" );
    TextLabel1_2_2_2_3_2_2->setGeometry( QRect( 521, 360, 20, 20 ) ); 
    TextLabel1_2_2_2_3_2_2->setText( tr( "r11" ) );
    TextLabel1_2_2_2_3_2_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbR5 = new QLineEdit( this, "tbR5" );
    tbR5->setGeometry( QRect( 550, 180, 100, 22 ) ); 
    tbR5->setText( tr( "" ) );

    TextLabel1_2_2_2_2_2_2_4_3 = new QLabel( this, "TextLabel1_2_2_2_2_2_2_4_3" );
    TextLabel1_2_2_2_2_2_2_4_3->setGeometry( QRect( 521, 210, 20, 20 ) ); 
    TextLabel1_2_2_2_2_2_2_4_3->setText( tr( "r6" ) );
    TextLabel1_2_2_2_2_2_2_4_3->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbR13 = new QLineEdit( this, "tbR13" );
    tbR13->setGeometry( QRect( 550, 420, 100, 22 ) ); 
    tbR13->setText( tr( "" ) );

    TextLabel1_2_3_2_2 = new QLabel( this, "TextLabel1_2_3_2_2" );
    TextLabel1_2_3_2_2->setGeometry( QRect( 520, 300, 20, 20 ) ); 
    TextLabel1_2_3_2_2->setText( tr( "r9" ) );
    TextLabel1_2_3_2_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_2_2_2_2_2_2_3_2_2 = new QLabel( this, "TextLabel1_2_2_2_2_2_2_3_2_2" );
    TextLabel1_2_2_2_2_2_2_3_2_2->setGeometry( QRect( 521, 450, 20, 20 ) ); 
    TextLabel1_2_2_2_2_2_2_3_2_2->setText( tr( "r14" ) );
    TextLabel1_2_2_2_2_2_2_3_2_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbCommxmit1 = new QLineEdit( this, "tbCommxmit1" );
    tbCommxmit1->setGeometry( QRect( 410, 300, 100, 22 ) ); 
    tbCommxmit1->setText( tr( "" ) );

    TextLabel1_3_2 = new QLabel( this, "TextLabel1_3_2" );
    TextLabel1_3_2->setGeometry( QRect( 201, 270, 20, 20 ) ); 
    TextLabel1_3_2->setText( tr( "rv" ) );
    TextLabel1_3_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbXybase = new QLineEdit( this, "tbXybase" );
    tbXybase->setGeometry( QRect( 230, 210, 100, 22 ) ); 
    tbXybase->setText( tr( "" ) );

    TextLabel1_2_2_2_2_2 = new QLabel( this, "TextLabel1_2_2_2_2_2" );
    TextLabel1_2_2_2_2_2->setGeometry( QRect( 21, 180, 40, 20 ) ); 
    TextLabel1_2_2_2_2_2->setText( tr( "pcroute" ) );
    TextLabel1_2_2_2_2_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_2_2_2_2_2_2_3 = new QLabel( this, "TextLabel1_2_2_2_2_2_2_3" );
    TextLabel1_2_2_2_2_2_2_3->setGeometry( QRect( 31, 450, 30, 20 ) ); 
    TextLabel1_2_2_2_2_2_2_3->setText( tr( "inten1" ) );
    TextLabel1_2_2_2_2_2_2_3->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbIntvec2 = new QLineEdit( this, "tbIntvec2" );
    tbIntvec2->setGeometry( QRect( 70, 360, 100, 22 ) ); 
    tbIntvec2->setText( tr( "" ) );

    TextLabel1_2_2_2_2_3_2_2_2 = new QLabel( this, "TextLabel1_2_2_2_2_3_2_2_2" );
    TextLabel1_2_2_2_2_3_2_2_2->setGeometry( QRect( 660, 390, 20, 20 ) ); 
    TextLabel1_2_2_2_2_3_2_2_2->setText( tr( "r28" ) );
    TextLabel1_2_2_2_2_3_2_2_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbR16 = new QLineEdit( this, "tbR16" );
    tbR16->setGeometry( QRect( 690, 30, 100, 22 ) ); 
    tbR16->setText( tr( "" ) );

    tbR9 = new QLineEdit( this, "tbR9" );
    tbR9->setGeometry( QRect( 550, 300, 100, 22 ) ); 
    tbR9->setText( tr( "" ) );

    TextLabel1_2_2_4_3 = new QLabel( this, "TextLabel1_2_2_4_3" );
    TextLabel1_2_2_4_3->setGeometry( QRect( 521, 90, 20, 20 ) ); 
    TextLabel1_2_2_4_3->setText( tr( "r2" ) );
    TextLabel1_2_2_4_3->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbR7 = new QLineEdit( this, "tbR7" );
    tbR7->setGeometry( QRect( 550, 240, 100, 22 ) ); 
    tbR7->setText( tr( "" ) );

    tbOdmactl = new QLineEdit( this, "tbOdmactl" );
    tbOdmactl->setGeometry( QRect( 410, 90, 100, 22 ) ); 
    tbOdmactl->setText( tr( "" ) );

    tbDabreak = new QLineEdit( this, "tbDabreak" );
    tbDabreak->setGeometry( QRect( 410, 60, 100, 22 ) ); 
    tbDabreak->setText( tr( "" ) );

    tbXyrange = new QLineEdit( this, "tbXyrange" );
    tbXyrange->setGeometry( QRect( 230, 180, 100, 22 ) ); 
    tbXyrange->setText( tr( "" ) );

    TextLabel1_2_2_2_2_2_2_2_3 = new QLabel( this, "TextLabel1_2_2_2_2_2_2_2_3" );
    TextLabel1_2_2_2_2_2_2_2_3->setGeometry( QRect( 201, 240, 20, 20 ) ); 
    TextLabel1_2_2_2_2_2_2_2_3->setText( tr( "ru" ) );
    TextLabel1_2_2_2_2_2_2_2_3->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbSvshift = new QLineEdit( this, "tbSvshift" );
    tbSvshift->setGeometry( QRect( 230, 450, 100, 22 ) ); 
    tbSvshift->setText( tr( "" ) );

    TextLabel1_2_2_2_2_2_2 = new QLabel( this, "TextLabel1_2_2_2_2_2_2" );
    TextLabel1_2_2_2_2_2_2->setGeometry( QRect( 21, 210, 40, 20 ) ); 
    TextLabel1_2_2_2_2_2_2->setText( tr( "pcexec" ) );
    TextLabel1_2_2_2_2_2_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbRz = new QLineEdit( this, "tbRz" );
    tbRz->setGeometry( QRect( 70, 240, 100, 22 ) ); 
    tbRz->setText( tr( "" ) );

    TextLabel1_2_2_2 = new QLabel( this, "TextLabel1_2_2_2" );
    TextLabel1_2_2_2->setGeometry( QRect( 41, 120, 20, 20 ) ); 
    TextLabel1_2_2_2->setText( tr( "cc" ) );
    TextLabel1_2_2_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_2_2_2_3_2_2_2 = new QLabel( this, "TextLabel1_2_2_2_3_2_2_2" );
    TextLabel1_2_2_2_3_2_2_2->setGeometry( QRect( 661, 360, 20, 20 ) ); 
    TextLabel1_2_2_2_3_2_2_2->setText( tr( "r27" ) );
    TextLabel1_2_2_2_3_2_2_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbR28 = new QLineEdit( this, "tbR28" );
    tbR28->setGeometry( QRect( 690, 390, 100, 22 ) ); 
    tbR28->setText( tr( "" ) );

    TextLabel1_2_2_2_4_3_2 = new QLabel( this, "TextLabel1_2_2_2_4_3_2" );
    TextLabel1_2_2_2_4_3_2->setGeometry( QRect( 661, 120, 20, 20 ) ); 
    TextLabel1_2_2_2_4_3_2->setText( tr( "r19" ) );
    TextLabel1_2_2_2_4_3_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_2_3_2_2_2 = new QLabel( this, "TextLabel1_2_3_2_2_2" );
    TextLabel1_2_3_2_2_2->setGeometry( QRect( 660, 300, 20, 20 ) ); 
    TextLabel1_2_3_2_2_2->setText( tr( "r25" ) );
    TextLabel1_2_3_2_2_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbR4 = new QLineEdit( this, "tbR4" );
    tbR4->setGeometry( QRect( 550, 150, 100, 22 ) ); 
    tbR4->setText( tr( "" ) );

    TextLabel1_3_2_2 = new QLabel( this, "TextLabel1_3_2_2" );
    TextLabel1_3_2_2->setGeometry( QRect( 340, 270, 60, 20 ) ); 
    TextLabel1_3_2_2->setText( tr( "commxmit0" ) );
    TextLabel1_3_2_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbXyctl = new QLineEdit( this, "tbXyctl" );
    tbXyctl->setGeometry( QRect( 230, 150, 100, 22 ) ); 
    tbXyctl->setText( tr( "" ) );

    TextLabel1_2_2_2_2_2_3 = new QLabel( this, "TextLabel1_2_2_2_2_2_3" );
    TextLabel1_2_2_2_2_2_3->setGeometry( QRect( 31, 420, 30, 20 ) ); 
    TextLabel1_2_2_2_2_2_3->setText( tr( "intctl" ) );
    TextLabel1_2_2_2_2_2_3->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_2_2_2_3 = new QLabel( this, "TextLabel1_2_2_2_3" );
    TextLabel1_2_2_2_3->setGeometry( QRect( 21, 360, 40, 20 ) ); 
    TextLabel1_2_2_2_3->setText( tr( "intvec2" ) );
    TextLabel1_2_2_2_3->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_2_3 = new QLabel( this, "TextLabel1_2_3" );
    TextLabel1_2_3->setGeometry( QRect( 41, 300, 20, 20 ) ); 
    TextLabel1_2_3->setText( tr( "rzi2" ) );
    TextLabel1_2_3->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_4 = new QLabel( this, "TextLabel1_4" );
    TextLabel1_4->setGeometry( QRect( 181, 30, 40, 20 ) ); 
    TextLabel1_4->setText( tr( "rc0" ) );
    TextLabel1_4->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbPcfetch = new QLineEdit( this, "tbPcfetch" );
    tbPcfetch->setGeometry( QRect( 70, 150, 100, 22 ) ); 
    tbPcfetch->setText( tr( "" ) );

    tbMpectl = new QLineEdit( this, "tbMpectl" );
    tbMpectl->setGeometry( QRect( 70, 30, 100, 22 ) ); 
    tbMpectl->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, tbMpectl->sizePolicy().hasHeightForWidth() ) );
    tbMpectl->setText( tr( "" ) );

    TextLabel1_2_4_3_2 = new QLabel( this, "TextLabel1_2_4_3_2" );
    TextLabel1_2_4_3_2->setGeometry( QRect( 660, 60, 20, 20 ) ); 
    TextLabel1_2_4_3_2->setText( tr( "r17" ) );
    TextLabel1_2_4_3_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_2_2_2_2_2_2_2_3_3_2 = new QLabel( this, "TextLabel1_2_2_2_2_2_2_2_3_3_2" );
    TextLabel1_2_2_2_2_2_2_2_3_3_2->setGeometry( QRect( 661, 240, 20, 20 ) ); 
    TextLabel1_2_2_2_2_2_2_2_3_3_2->setText( tr( "r23" ) );
    TextLabel1_2_2_2_2_2_2_2_3_3_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbR19 = new QLineEdit( this, "tbR19" );
    tbR19->setGeometry( QRect( 690, 120, 100, 22 ) ); 
    tbR19->setText( tr( "" ) );

    tbR17 = new QLineEdit( this, "tbR17" );
    tbR17->setGeometry( QRect( 690, 60, 100, 22 ) ); 
    tbR17->setText( tr( "" ) );

    TextLabel1_2_2_2_2_2_3_2_2 = new QLabel( this, "TextLabel1_2_2_2_2_2_3_2_2" );
    TextLabel1_2_2_2_2_2_3_2_2->setGeometry( QRect( 520, 420, 20, 20 ) ); 
    TextLabel1_2_2_2_2_2_3_2_2->setText( tr( "r13" ) );
    TextLabel1_2_2_2_2_2_3_2_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbR12 = new QLineEdit( this, "tbR12" );
    tbR12->setGeometry( QRect( 550, 390, 100, 22 ) ); 
    tbR12->setText( tr( "" ) );

    TextLabel1_2_2_2_2_4_3 = new QLabel( this, "TextLabel1_2_2_2_2_4_3" );
    TextLabel1_2_2_2_2_4_3->setGeometry( QRect( 521, 150, 20, 20 ) ); 
    TextLabel1_2_2_2_2_4_3->setText( tr( "r4" ) );
    TextLabel1_2_2_2_2_4_3->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_3_2_3 = new QLabel( this, "TextLabel1_3_2_3" );
    TextLabel1_3_2_3->setGeometry( QRect( 521, 270, 20, 20 ) ); 
    TextLabel1_3_2_3->setText( tr( "r8" ) );
    TextLabel1_3_2_3->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_2_4_2 = new QLabel( this, "TextLabel1_2_4_2" );
    TextLabel1_2_4_2->setGeometry( QRect( 351, 60, 50, 20 ) ); 
    TextLabel1_2_4_2->setText( tr( "dabreak" ) );
    TextLabel1_2_4_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbMdmactl = new QLineEdit( this, "tbMdmactl" );
    tbMdmactl->setGeometry( QRect( 410, 150, 100, 22 ) ); 
    tbMdmactl->setText( tr( "" ) );

    TextLabel1_2_2_2_2_2_3_2 = new QLabel( this, "TextLabel1_2_2_2_2_2_3_2" );
    TextLabel1_2_2_2_2_2_3_2->setGeometry( QRect( 180, 420, 40, 20 ) ); 
    TextLabel1_2_2_2_2_2_3_2->setText( tr( "clutbase" ) );
    TextLabel1_2_2_2_2_2_3_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbUvctl = new QLineEdit( this, "tbUvctl" );
    tbUvctl->setGeometry( QRect( 230, 360, 100, 22 ) ); 
    tbUvctl->setText( tr( "" ) );

    tbIntvec1 = new QLineEdit( this, "tbIntvec1" );
    tbIntvec1->setGeometry( QRect( 70, 330, 100, 22 ) ); 
    tbIntvec1->setText( tr( "" ) );

    tbR25 = new QLineEdit( this, "tbR25" );
    tbR25->setGeometry( QRect( 690, 300, 100, 22 ) ); 
    tbR25->setText( tr( "" ) );

    tbR24 = new QLineEdit( this, "tbR24" );
    tbR24->setGeometry( QRect( 690, 270, 100, 22 ) ); 
    tbR24->setText( tr( "" ) );

    tbR29 = new QLineEdit( this, "tbR29" );
    tbR29->setGeometry( QRect( 690, 420, 100, 22 ) ); 
    tbR29->setText( tr( "" ) );

    tbR6 = new QLineEdit( this, "tbR6" );
    tbR6->setGeometry( QRect( 550, 210, 100, 22 ) ); 
    tbR6->setText( tr( "" ) );

    tbR10 = new QLineEdit( this, "tbR10" );
    tbR10->setGeometry( QRect( 550, 330, 100, 22 ) ); 
    tbR10->setText( tr( "" ) );

    tbR11 = new QLineEdit( this, "tbR11" );
    tbR11->setGeometry( QRect( 550, 360, 100, 22 ) ); 
    tbR11->setText( tr( "" ) );

    tbR3 = new QLineEdit( this, "tbR3" );
    tbR3->setGeometry( QRect( 550, 120, 100, 22 ) ); 
    tbR3->setText( tr( "" ) );

    tbComminfo = new QLineEdit( this, "tbComminfo" );
    tbComminfo->setGeometry( QRect( 410, 210, 100, 22 ) ); 
    tbComminfo->setText( tr( "" ) );

    tbCommrecv3 = new QLineEdit( this, "tbCommrecv3" );
    tbCommrecv3->setGeometry( QRect( 410, 480, 100, 22 ) ); 
    tbCommrecv3->setText( tr( "" ) );

    TextLabel1_3_2_2_3_2 = new QLabel( this, "TextLabel1_3_2_2_3_2" );
    TextLabel1_3_2_2_3_2->setGeometry( QRect( 340, 450, 60, 20 ) ); 
    TextLabel1_3_2_2_3_2->setText( tr( "commrecv2" ) );
    TextLabel1_3_2_2_3_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbOdmacptr = new QLineEdit( this, "tbOdmacptr" );
    tbOdmacptr->setGeometry( QRect( 410, 120, 100, 22 ) ); 
    tbOdmacptr->setText( tr( "" ) );

    TextLabel1_2_2_2_2_2_2_2 = new QLabel( this, "TextLabel1_2_2_2_2_2_2_2" );
    TextLabel1_2_2_2_2_2_2_2->setGeometry( QRect( 41, 240, 20, 20 ) ); 
    TextLabel1_2_2_2_2_2_2_2->setText( tr( "rz" ) );
    TextLabel1_2_2_2_2_2_2_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbPcexec = new QLineEdit( this, "tbPcexec" );
    tbPcexec->setGeometry( QRect( 70, 210, 100, 22 ) ); 
    tbPcexec->setText( tr( "" ) );

    tbCc = new QLineEdit( this, "tbCc" );
    tbCc->setGeometry( QRect( 70, 120, 100, 22 ) ); 
    tbCc->setText( tr( "" ) );

    TextLabel1_3_2_2_4 = new QLabel( this, "TextLabel1_3_2_2_4" );
    TextLabel1_3_2_2_4->setGeometry( QRect( 340, 360, 60, 20 ) ); 
    TextLabel1_3_2_2_4->setText( tr( "commxmit3" ) );
    TextLabel1_3_2_2_4->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbSp = new QLineEdit( this, "tbSp" );
    tbSp->setGeometry( QRect( 410, 30, 100, 22 ) ); 
    tbSp->setText( tr( "" ) );

    tbUvbase = new QLineEdit( this, "tbUvbase" );
    tbUvbase->setGeometry( QRect( 230, 330, 100, 22 ) ); 
    tbUvbase->setText( tr( "" ) );

    tbInten2sel = new QLineEdit( this, "tbInten2sel" );
    tbInten2sel->setGeometry( QRect( 70, 480, 100, 22 ) ); 
    tbInten2sel->setText( tr( "" ) );

    tbRzi2 = new QLineEdit( this, "tbRzi2" );
    tbRzi2->setGeometry( QRect( 70, 300, 100, 22 ) ); 
    tbRzi2->setText( tr( "" ) );

    tbR18 = new QLineEdit( this, "tbR18" );
    tbR18->setGeometry( QRect( 690, 90, 100, 22 ) ); 
    tbR18->setText( tr( "" ) );

    tbR22 = new QLineEdit( this, "tbR22" );
    tbR22->setGeometry( QRect( 690, 210, 100, 22 ) ); 
    tbR22->setText( tr( "" ) );

    tbR20 = new QLineEdit( this, "tbR20" );
    tbR20->setGeometry( QRect( 690, 150, 100, 22 ) ); 
    tbR20->setText( tr( "" ) );

    tbR0 = new QLineEdit( this, "tbR0" );
    tbR0->setGeometry( QRect( 550, 30, 100, 22 ) ); 
    tbR0->setText( tr( "" ) );

    tbCommxmit2 = new QLineEdit( this, "tbCommxmit2" );
    tbCommxmit2->setGeometry( QRect( 410, 330, 100, 22 ) ); 
    tbCommxmit2->setText( tr( "" ) );

    TextLabel1_2_2_2_2_4 = new QLabel( this, "TextLabel1_2_2_2_2_4" );
    TextLabel1_2_2_2_2_4->setGeometry( QRect( 201, 150, 20, 20 ) ); 
    TextLabel1_2_2_2_2_4->setText( tr( "xyctl" ) );
    TextLabel1_2_2_2_2_4->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbRy = new QLineEdit( this, "tbRy" );
    tbRy->setGeometry( QRect( 230, 120, 100, 22 ) ); 
    tbRy->setText( tr( "" ) );

    TextLabel1_2_2_2_4 = new QLabel( this, "TextLabel1_2_2_2_4" );
    TextLabel1_2_2_2_4->setGeometry( QRect( 201, 120, 20, 20 ) ); 
    TextLabel1_2_2_2_4->setText( tr( "ry" ) );
    TextLabel1_2_2_2_4->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_2_2_2_2_3 = new QLabel( this, "TextLabel1_2_2_2_2_3" );
    TextLabel1_2_2_2_2_3->setGeometry( QRect( 31, 390, 30, 20 ) ); 
    TextLabel1_2_2_2_2_3->setText( tr( "intsrc" ) );
    TextLabel1_2_2_2_2_3->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbRc1 = new QLineEdit( this, "tbRc1" );
    tbRc1->setGeometry( QRect( 230, 60, 100, 22 ) ); 
    tbRc1->setText( tr( "" ) );

    TextLabel1_2_2 = new QLabel( this, "TextLabel1_2_2" );
    TextLabel1_2_2->setGeometry( QRect( 1, 90, 60, 20 ) ); 
    TextLabel1_2_2->setText( tr( "excephalten" ) );
    TextLabel1_2_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbR26 = new QLineEdit( this, "tbR26" );
    tbR26->setGeometry( QRect( 690, 330, 100, 22 ) ); 
    tbR26->setText( tr( "" ) );

    tbR21 = new QLineEdit( this, "tbR21" );
    tbR21->setGeometry( QRect( 690, 180, 100, 22 ) ); 
    tbR21->setText( tr( "" ) );

    TextLabel1_2_2_2_4_3 = new QLabel( this, "TextLabel1_2_2_2_4_3" );
    TextLabel1_2_2_2_4_3->setGeometry( QRect( 521, 120, 20, 20 ) ); 
    TextLabel1_2_2_2_4_3->setText( tr( "r3" ) );
    TextLabel1_2_2_2_4_3->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_2_2_2_2_2_2_2_2_2_2 = new QLabel( this, "TextLabel1_2_2_2_2_2_2_2_2_2_2" );
    TextLabel1_2_2_2_2_2_2_2_2_2_2->setGeometry( QRect( 521, 480, 20, 20 ) ); 
    TextLabel1_2_2_2_2_2_2_2_2_2_2->setText( tr( "r15" ) );
    TextLabel1_2_2_2_2_2_2_2_2_2_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_3_2_2_2_2 = new QLabel( this, "TextLabel1_3_2_2_2_2" );
    TextLabel1_3_2_2_2_2->setGeometry( QRect( 340, 420, 60, 20 ) ); 
    TextLabel1_3_2_2_2_2->setText( tr( "commrecv1" ) );
    TextLabel1_3_2_2_2_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbLinpixctl = new QLineEdit( this, "tbLinpixctl" );
    tbLinpixctl->setGeometry( QRect( 230, 390, 100, 22 ) ); 
    tbLinpixctl->setText( tr( "" ) );

    TextLabel1_2_4 = new QLabel( this, "TextLabel1_2_4" );
    TextLabel1_2_4->setGeometry( QRect( 201, 60, 20, 20 ) ); 
    TextLabel1_2_4->setText( tr( "rc1" ) );
    TextLabel1_2_4->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_2_2_3 = new QLabel( this, "TextLabel1_2_2_3" );
    TextLabel1_2_2_3->setGeometry( QRect( 21, 330, 40, 20 ) ); 
    TextLabel1_2_2_3->setText( tr( "intvec1" ) );
    TextLabel1_2_2_3->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbRc0 = new QLineEdit( this, "tbRc0" );
    tbRc0->setGeometry( QRect( 230, 30, 100, 22 ) ); 
    tbRc0->setText( tr( "" ) );

    tbRzi1 = new QLineEdit( this, "tbRzi1" );
    tbRzi1->setGeometry( QRect( 70, 270, 100, 22 ) ); 
    tbRzi1->setText( tr( "" ) );

    tbExcephalten = new QLineEdit( this, "tbExcephalten" );
    tbExcephalten->setGeometry( QRect( 70, 90, 100, 22 ) ); 
    tbExcephalten->setText( tr( "" ) );

    TextLabel1_2_2_2_2_2_3_2_2_2 = new QLabel( this, "TextLabel1_2_2_2_2_2_3_2_2_2" );
    TextLabel1_2_2_2_2_2_3_2_2_2->setGeometry( QRect( 660, 420, 20, 20 ) ); 
    TextLabel1_2_2_2_2_2_3_2_2_2->setText( tr( "r29" ) );
    TextLabel1_2_2_2_2_2_3_2_2_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbCommxmit3 = new QLineEdit( this, "tbCommxmit3" );
    tbCommxmit3->setGeometry( QRect( 410, 360, 100, 22 ) ); 
    tbCommxmit3->setText( tr( "" ) );

    TextLabel1_3_2_2_5 = new QLabel( this, "TextLabel1_3_2_2_5" );
    TextLabel1_3_2_2_5->setGeometry( QRect( 340, 390, 60, 20 ) ); 
    TextLabel1_3_2_2_5->setText( tr( "commrecv0" ) );
    TextLabel1_3_2_2_5->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_4_2 = new QLabel( this, "TextLabel1_4_2" );
    TextLabel1_4_2->setGeometry( QRect( 361, 30, 40, 20 ) ); 
    TextLabel1_4_2->setText( tr( "sp" ) );
    TextLabel1_4_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbCommrecv1 = new QLineEdit( this, "tbCommrecv1" );
    tbCommrecv1->setGeometry( QRect( 410, 420, 100, 22 ) ); 
    tbCommrecv1->setText( tr( "" ) );

    tbUvrange = new QLineEdit( this, "tbUvrange" );
    tbUvrange->setGeometry( QRect( 230, 300, 100, 22 ) ); 
    tbUvrange->setText( tr( "" ) );

    tbRx = new QLineEdit( this, "tbRx" );
    tbRx->setGeometry( QRect( 230, 90, 100, 22 ) ); 
    tbRx->setText( tr( "" ) );

    tbInten1 = new QLineEdit( this, "tbInten1" );
    tbInten1->setGeometry( QRect( 70, 450, 100, 22 ) ); 
    tbInten1->setText( tr( "" ) );

    TextLabel1_3_2_2_3 = new QLabel( this, "TextLabel1_3_2_2_3" );
    TextLabel1_3_2_2_3->setGeometry( QRect( 340, 330, 60, 20 ) ); 
    TextLabel1_3_2_2_3->setText( tr( "commxmit2" ) );
    TextLabel1_3_2_2_3->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbCommctl = new QLineEdit( this, "tbCommctl" );
    tbCommctl->setGeometry( QRect( 410, 240, 100, 22 ) ); 
    tbCommctl->setText( tr( "" ) );

    TextLabel1_2_2_2_2_3_2 = new QLabel( this, "TextLabel1_2_2_2_2_3_2" );
    TextLabel1_2_2_2_2_3_2->setGeometry( QRect( 181, 390, 40, 20 ) ); 
    TextLabel1_2_2_2_2_3_2->setText( tr( "linpixctl" ) );
    TextLabel1_2_2_2_2_3_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbMdmacptr = new QLineEdit( this, "tbMdmacptr" );
    tbMdmacptr->setGeometry( QRect( 410, 180, 100, 22 ) ); 
    tbMdmacptr->setText( tr( "" ) );

    TextLabel1_3_2_2_2 = new QLabel( this, "TextLabel1_3_2_2_2" );
    TextLabel1_3_2_2_2->setGeometry( QRect( 340, 300, 60, 20 ) ); 
    TextLabel1_3_2_2_2->setText( tr( "commxmit1" ) );
    TextLabel1_3_2_2_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_2_2_2_2_2_2_2_2_2 = new QLabel( this, "TextLabel1_2_2_2_2_2_2_2_2_2" );
    TextLabel1_2_2_2_2_2_2_2_2_2->setGeometry( QRect( 191, 480, 30, 20 ) ); 
    TextLabel1_2_2_2_2_2_2_2_2_2->setText( tr( "acshift" ) );
    TextLabel1_2_2_2_2_2_2_2_2_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_2_2_2_2_2_2_2_3_3 = new QLabel( this, "TextLabel1_2_2_2_2_2_2_2_3_3" );
    TextLabel1_2_2_2_2_2_2_2_3_3->setGeometry( QRect( 521, 240, 20, 20 ) ); 
    TextLabel1_2_2_2_2_2_2_2_3_3->setText( tr( "r7" ) );
    TextLabel1_2_2_2_2_2_2_2_3_3->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_2_2_2_2_3_2_2 = new QLabel( this, "TextLabel1_2_2_2_2_3_2_2" );
    TextLabel1_2_2_2_2_3_2_2->setGeometry( QRect( 521, 390, 20, 20 ) ); 
    TextLabel1_2_2_2_2_3_2_2->setText( tr( "r12" ) );
    TextLabel1_2_2_2_2_3_2_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbR1 = new QLineEdit( this, "tbR1" );
    tbR1->setGeometry( QRect( 550, 60, 100, 22 ) ); 
    tbR1->setText( tr( "" ) );

    TextLabel1_2_4_3 = new QLabel( this, "TextLabel1_2_4_3" );
    TextLabel1_2_4_3->setGeometry( QRect( 520, 60, 20, 20 ) ); 
    TextLabel1_2_4_3->setText( tr( "r1" ) );
    TextLabel1_2_4_3->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_2_2_2_2_2_2_4_2 = new QLabel( this, "TextLabel1_2_2_2_2_2_2_4_2" );
    TextLabel1_2_2_2_2_2_2_4_2->setGeometry( QRect( 351, 210, 50, 20 ) ); 
    TextLabel1_2_2_2_2_2_2_4_2->setText( tr( "comminfo" ) );
    TextLabel1_2_2_2_2_2_2_4_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_2_2_2_4_2 = new QLabel( this, "TextLabel1_2_2_2_4_2" );
    TextLabel1_2_2_2_4_2->setGeometry( QRect( 351, 120, 50, 20 ) ); 
    TextLabel1_2_2_2_4_2->setText( tr( "odmacptr" ) );
    TextLabel1_2_2_2_4_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbCommrecv2 = new QLineEdit( this, "tbCommrecv2" );
    tbCommrecv2->setGeometry( QRect( 410, 450, 100, 22 ) ); 
    tbCommrecv2->setText( tr( "" ) );

    tbCommrecv0 = new QLineEdit( this, "tbCommrecv0" );
    tbCommrecv0->setGeometry( QRect( 410, 390, 100, 22 ) ); 
    tbCommrecv0->setText( tr( "" ) );

    TextLabel1_2_2_2_2 = new QLabel( this, "TextLabel1_2_2_2_2" );
    TextLabel1_2_2_2_2->setGeometry( QRect( 21, 150, 40, 20 ) ); 
    TextLabel1_2_2_2_2->setText( tr( "pcfetch" ) );
    TextLabel1_2_2_2_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbIntctl = new QLineEdit( this, "tbIntctl" );
    tbIntctl->setGeometry( QRect( 70, 420, 100, 22 ) ); 
    tbIntctl->setText( tr( "" ) );

    tbR30 = new QLineEdit( this, "tbR30" );
    tbR30->setGeometry( QRect( 690, 450, 100, 22 ) ); 
    tbR30->setText( tr( "" ) );

    TextLabel1_2 = new QLabel( this, "TextLabel1_2" );
    TextLabel1_2->setGeometry( QRect( 11, 60, 50, 20 ) ); 
    TextLabel1_2->setText( tr( "excepsrc" ) );
    TextLabel1_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_2_2_2_2_2_2_3_2_2_2 = new QLabel( this, "TextLabel1_2_2_2_2_2_2_3_2_2_2" );
    TextLabel1_2_2_2_2_2_2_3_2_2_2->setGeometry( QRect( 661, 450, 20, 20 ) ); 
    TextLabel1_2_2_2_2_2_2_3_2_2_2->setText( tr( "r30" ) );
    TextLabel1_2_2_2_2_2_2_3_2_2_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_2_2_2_2_4_3_2 = new QLabel( this, "TextLabel1_2_2_2_2_4_3_2" );
    TextLabel1_2_2_2_2_4_3_2->setGeometry( QRect( 661, 150, 20, 20 ) ); 
    TextLabel1_2_2_2_2_4_3_2->setText( tr( "r20" ) );
    TextLabel1_2_2_2_2_4_3_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbR15 = new QLineEdit( this, "tbR15" );
    tbR15->setGeometry( QRect( 550, 480, 100, 22 ) ); 
    tbR15->setText( tr( "" ) );

    TextLabel1_2_2_2_2_2_2_2_3_2 = new QLabel( this, "TextLabel1_2_2_2_2_2_2_2_3_2" );
    TextLabel1_2_2_2_2_2_2_2_3_2->setGeometry( QRect( 361, 240, 40, 20 ) ); 
    TextLabel1_2_2_2_2_2_2_2_3_2->setText( tr( "commctl" ) );
    TextLabel1_2_2_2_2_2_2_2_3_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbClutbase = new QLineEdit( this, "tbClutbase" );
    tbClutbase->setGeometry( QRect( 230, 420, 100, 22 ) ); 
    tbClutbase->setText( tr( "" ) );

    TextLabel1_3 = new QLabel( this, "TextLabel1_3" );
    TextLabel1_3->setGeometry( QRect( 41, 270, 20, 20 ) ); 
    TextLabel1_3->setText( tr( "rzi1" ) );
    TextLabel1_3->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_2_2_2_2_2_2_2_2 = new QLabel( this, "TextLabel1_2_2_2_2_2_2_2_2" );
    TextLabel1_2_2_2_2_2_2_2_2->setGeometry( QRect( 11, 480, 50, 20 ) ); 
    TextLabel1_2_2_2_2_2_2_2_2->setText( tr( "inten2sel" ) );
    TextLabel1_2_2_2_2_2_2_2_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_4_3_2 = new QLabel( this, "TextLabel1_4_3_2" );
    TextLabel1_4_3_2->setGeometry( QRect( 660, 30, 20, 20 ) ); 
    TextLabel1_4_3_2->setText( tr( "r16" ) );
    TextLabel1_4_3_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_2_2_2_2_2_2_4_3_2 = new QLabel( this, "TextLabel1_2_2_2_2_2_2_4_3_2" );
    TextLabel1_2_2_2_2_2_2_4_3_2->setGeometry( QRect( 661, 210, 20, 20 ) ); 
    TextLabel1_2_2_2_2_2_2_4_3_2->setText( tr( "r22" ) );
    TextLabel1_2_2_2_2_2_2_4_3_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbR27 = new QLineEdit( this, "tbR27" );
    tbR27->setGeometry( QRect( 690, 360, 100, 22 ) ); 
    tbR27->setText( tr( "" ) );

    TextLabel1_4_3 = new QLabel( this, "TextLabel1_4_3" );
    TextLabel1_4_3->setGeometry( QRect( 520, 30, 20, 20 ) ); 
    TextLabel1_4_3->setText( tr( "r0" ) );
    TextLabel1_4_3->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_2_2_2_2_2_2_4 = new QLabel( this, "TextLabel1_2_2_2_2_2_2_4" );
    TextLabel1_2_2_2_2_2_2_4->setGeometry( QRect( 181, 210, 40, 20 ) ); 
    TextLabel1_2_2_2_2_2_2_4->setText( tr( "xybase" ) );
    TextLabel1_2_2_2_2_2_2_4->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    tbAcshift = new QLineEdit( this, "tbAcshift" );
    tbAcshift->setGeometry( QRect( 230, 480, 100, 22 ) ); 
    tbAcshift->setText( tr( "" ) );

    TextLabel2 = new QLabel( this, "TextLabel2" );
    TextLabel2->setGeometry( QRect( 20, 520, 40, 20 ) ); 
    TextLabel2->setText( tr( "memory" ) );

    tbIntsrc = new QLineEdit( this, "tbIntsrc" );
    tbIntsrc->setGeometry( QRect( 70, 390, 100, 22 ) ); 
    tbIntsrc->setText( tr( "" ) );

    TextLabel1_2_2_2_2_2_2_2_2_2_2_2 = new QLabel( this, "TextLabel1_2_2_2_2_2_2_2_2_2_2_2" );
    TextLabel1_2_2_2_2_2_2_2_2_2_2_2->setGeometry( QRect( 660, 480, 20, 20 ) ); 
    TextLabel1_2_2_2_2_2_2_2_2_2_2_2->setText( tr( "r31" ) );
    TextLabel1_2_2_2_2_2_2_2_2_2_2_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    TextLabel1_5 = new QLabel( this, "TextLabel1_5" );
    TextLabel1_5->setGeometry( QRect( 600, 520, 120, 16 ) ); 
    TextLabel1_5->setText( tr( "Memory Bank Select" ) );

    ddlbSelectMemoryBank = new QComboBox( FALSE, this, "ddlbSelectMemoryBank" );
    ddlbSelectMemoryBank->insertItem( tr( "MPE Local Memory" ) );
    ddlbSelectMemoryBank->insertItem( tr( "Main Bus RAM" ) );
    ddlbSelectMemoryBank->insertItem( tr( "System Bus RAM" ) );
    ddlbSelectMemoryBank->setGeometry( QRect( 600, 540, 120, 20 ) ); 

    TextLabel1_5_2 = new QLabel( this, "TextLabel1_5_2" );
    TextLabel1_5_2->setGeometry( QRect( 600, 680, 110, 16 ) ); 
    TextLabel1_5_2->setText( tr( "Memory Page Control" ) );

    sbMemoryPageLowerNibble = new QSpinBox( this, "sbMemoryPageLowerNibble" );
    sbMemoryPageLowerNibble->setGeometry( QRect( 650, 710, 30, 40 ) ); 
    sbMemoryPageLowerNibble->setWrapping( TRUE );
    sbMemoryPageLowerNibble->setButtonSymbols( QSpinBox::UpDownArrows );
    sbMemoryPageLowerNibble->setMaxValue( 15 );

    sbMemoryPageUpperNibble = new QSpinBox( this, "sbMemoryPageUpperNibble" );
    sbMemoryPageUpperNibble->setGeometry( QRect( 610, 710, 30, 40 ) ); 
    sbMemoryPageUpperNibble->setWrapping( TRUE );
    sbMemoryPageUpperNibble->setButtonSymbols( QSpinBox::UpDownArrows );
    sbMemoryPageUpperNibble->setMaxValue( 7 );

    tvMemory = new QTextView( this, "tvMemory" );
    tvMemory->setGeometry( QRect( 70, 520, 520, 230 ) ); 
    QFont tvMemory_font(  tvMemory->font() );
    tvMemory_font.setFamily( "Fixedsys" );
    tvMemory->setFont( tvMemory_font ); 
    tvMemory->setLineWidth( 2 );
    tvMemory->setVScrollBarMode( QTextView::Auto );
    tvMemory->setHScrollBarMode( QTextView::Auto );
    tvMemory->setText( tr( "" ) );
    tvMemory->setTextFormat( QTextView::PlainText );

    // signals and slots connections
    connect( sbMemoryPageLowerNibble, SIGNAL( valueChanged(int) ), this, SLOT( OnMemoryPageChange(int) ) );
    connect( ddlbSelectMemoryBank, SIGNAL( activated(int) ), this, SLOT( OnMemoryBankSelect(int) ) );
    connect( sbMemoryPageUpperNibble, SIGNAL( valueChanged(int) ), this, SLOT( OnMemoryPageChange(int) ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
RegisterWindow::~RegisterWindow()
{
    // no need to delete child widgets, Qt does it all for us
}

/*  
 *  Main event handler. Reimplemented to handle application
 *  font changes
 */
bool RegisterWindow::event( QEvent* ev )
{
    bool ret = QDialog::event( ev ); 
    if ( ev->type() == QEvent::ApplicationFontChange ) {
	QFont tvMemory_font(  tvMemory->font() );
	tvMemory_font.setFamily( "Fixedsys" );
	tvMemory->setFont( tvMemory_font ); 
    }
    return ret;
}

void RegisterWindow::OnMemoryBankSelect(int)
{
    qWarning( "RegisterWindow::OnMemoryBankSelect(int): Not implemented yet!" );
}

void RegisterWindow::OnMemoryPageChange(int)
{
    qWarning( "RegisterWindow::OnMemoryPageChange(int): Not implemented yet!" );
}

