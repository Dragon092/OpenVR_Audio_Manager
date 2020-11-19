#include "overlaywidget.h"
#include "openvroverlaycontroller.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName("GameMods");
    a.setOrganizationDomain("https://GameMods.org");
    a.setApplicationName("OVRAudioManager");

    OverlayWidget *pOverlayWidget = new OverlayWidget;

    COpenVROverlayController::SharedInstance()->Init();

    COpenVROverlayController::SharedInstance()->SetWidget( pOverlayWidget );

    // don't show widgets that you're going display in an overlay
    //w.show();

    return a.exec();
}
