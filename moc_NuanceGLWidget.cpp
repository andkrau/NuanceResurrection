/****************************************************************************
** NuanceGLWidget meta object code from reading C++ file 'NuanceGLWidget.h'
**
** Created: Sun Nov 7 23:54:24 2004
**      by: The Qt MOC ($Id: //depot/qt/main/src/moc/moc.y#178 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_NuanceGLWidget
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 8
#elif Q_MOC_OUTPUT_REVISION != 8
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "NuanceGLWidget.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if defined(Q_SPARCWORKS_FUNCP_BUG)
#define Q_AMPERSAND
#else
#define Q_AMPERSAND &
#endif


const char *NuanceGLWidget::className() const
{
    return "NuanceGLWidget";
}

QMetaObject *NuanceGLWidget::metaObj = 0;

void NuanceGLWidget::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QGLWidget::className(), "QGLWidget") != 0 )
	badSuperclassWarning("NuanceGLWidget","QGLWidget");
    (void) staticMetaObject();
}

#ifndef QT_NO_TRANSLATION
QString NuanceGLWidget::tr(const char* s)
{
    return ((QNonBaseApplication*)qApp)->translate("NuanceGLWidget",s);
}

#endif // QT_NO_TRANSLATION
QMetaObject* NuanceGLWidget::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) QGLWidget::staticMetaObject();
#ifndef QT_NO_PROPERTIES
#endif // QT_NO_PROPERTIES
    QMetaData::Access *slot_tbl_access = 0;
    metaObj = QMetaObject::new_metaobject(
	"NuanceGLWidget", "QGLWidget",
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
