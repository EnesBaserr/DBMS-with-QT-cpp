#include "mainwindow.h"
#include <QApplication>
#include <QIcon>
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("Dbms App");
    qApp->setStyleSheet(
        "QComboBox {"
        "  background-color: lightblue; "
        "  color: darkblue; "
        "  border: 1px solid gray; "
        "  padding: 2px; "
        "  font-size: 14px; "
        "  font-weight: bold; "
        "}"
        "QComboBox::drop-down {"
        "  subcontrol-origin: padding; "
        "  subcontrol-position: top right; "
        "  width: 30px; "
        "  border-left: 1px solid darkgray; "
        "}"
        "QComboBox::down-arrow {"
        "  image: url(:/arrow.png); "  // Path to the arrow image
        "  width: 16px; "  // Set the width of the arrow
        "  height: 16px; "  // Set the height of the arrow
        "}"
        "QComboBox QAbstractItemView {"
        "  background-color: orange; "
        "  color: black; "
        "  selection-background-color: grey; "
        "  selection-color: blue; "
        "}");


    MainWindow window;
     window.setWindowIcon(QIcon(":/data-server.png"));
    window.setWindowTitle(QApplication::applicationName());
    window.show();
    return app.exec();
}
