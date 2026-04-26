#include <QApplication>
#include "core/main_window.h"
#include "core/config.h"
#include "core/logger.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("MultiTool");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("MultiTool");

    Config::getInstance();
    Logger::instance();
    LOG_INFO("MultiTool application started");

    MainWindow window;
    window.show();

    int result = app.exec();
    LOG_INFO("MultiTool application exiting with code: " + QString::number(result));
    return result;
}
