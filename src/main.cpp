#include <QApplication>
#include <QTextCodec>
#include "core/main_window.h"
#include "core/config.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("MultiTool");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("MultiTool");

    Config::getInstance();

    MainWindow window;
    window.show();

    return app.exec();
}
