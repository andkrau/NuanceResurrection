/****************************************************************************
** NuanceRegisterWindow meta object code from reading C++ file 'NuanceRegisterWindow.h'
**
** Created: Sun Nov 7 23:54:24 2004
**      by: The Qt MOC ($Id: //depot/qt/main/src/moc/moc.y#178 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_NuanceRegisterWindow
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 8
#elif Q_MOC_OUTPUT_REVISION != 8
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "NuanceRegisterWindow.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if defined(Q_SPARCWORKS_FUNCP_BUG)
#define Q_AMPERSAND
#else
#define Q_AMPERSAND &
#endif


const char *NuanceRegisterWindow::className() const
{
    return "NuanceRegisterWindow";
}

QMetaObject *NuanceRegisterWindow::metaObj = 0;

void NuanceRegisterWindow::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(RegisterWindow::className(), "RegisterWindow") != 0 )
	badSuperclassWarning("NuanceRegisterWindow","RegisterWindow");
    (void) staticMetaObject();
}

#ifndef QT_NO_TRANSLATION
QString NuanceRegisterWindow::tr(const char* s)
{
    return ((QNonBaseApplication*)qApp)->translate("NuanceRegisterWindow",s);
}

#endif // QT_NO_TRANSLATION
QMetaObject* NuanceRegisterWindow::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) RegisterWindow::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    typedef void(NuanceRegisterWindow::*m1_t0)(int);
    typedef void(NuanceRegisterWindow::*m1_t1)(int);
    m1_t0 v1_0 = Q_AMPERSAND NuanceRegisterWindow::OnMemoryBankSelect;
    m1_t1 v1_1 = Q_AMPERSAND NuanceRegisterWindow::OnMemoryPageChange;
    QMetaData *slot_tbl = QMetaObject::new_metadata(2);
    QMetaData::Access *slot_tbl_access = QMetaObject::new_metaaccess(2);
    slot_tbl[0].name = "OnMemoryBankSelect(int)";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    slot_tbl_access[0] = QMetaData::Public;
    slot_tbl[1].name = "OnMemoryPageChange(int)";
    slot_tbl[1].ptr = *((QMember*)&v1_1);
    slot_tbl_access[1] = QMetaData::Public;
    metaObj = QMetaObject::new_metaobject(
	"NuanceRegisterWindow", "RegisterWindow",
	slot_tbl, 2,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    metaObj->set_slot_access( slot_tbl_access );
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    return metaObj;
}
