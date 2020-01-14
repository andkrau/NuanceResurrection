/****************************************************************************
** NuanceVideoDisplay meta object code from reading C++ file 'NuanceVideoDisplay.h'
**
** Created: Sun Nov 7 23:54:24 2004
**      by: The Qt MOC ($Id: //depot/qt/main/src/moc/moc.y#178 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_NuanceVideoDisplay
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 8
#elif Q_MOC_OUTPUT_REVISION != 8
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "NuanceVideoDisplay.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if defined(Q_SPARCWORKS_FUNCP_BUG)
#define Q_AMPERSAND
#else
#define Q_AMPERSAND &
#endif


const char *NuanceVideoDisplay::className() const
{
    return "NuanceVideoDisplay";
}

QMetaObject *NuanceVideoDisplay::metaObj = 0;

void NuanceVideoDisplay::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QDialog::className(), "QDialog") != 0 )
	badSuperclassWarning("NuanceVideoDisplay","QDialog");
    (void) staticMetaObject();
}

#ifndef QT_NO_TRANSLATION
QString NuanceVideoDisplay::tr(const char* s)
{
    return ((QNonBaseApplication*)qApp)->translate("NuanceVideoDisplay",s);
}

#endif // QT_NO_TRANSLATION
QMetaObject* NuanceVideoDisplay::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) QDialog::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    QMetaData::Access *slot_tbl_access = 0;
    metaObj = QMetaObject::new_metaobject(
	"NuanceVideoDisplay", "QDialog",
	0, 0,
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
