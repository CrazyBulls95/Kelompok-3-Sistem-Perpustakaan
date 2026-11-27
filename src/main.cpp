
#include <QApplication>
#include "dashboard.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    DashboardWindow dashboard;

    dashboard.show();

    return app.exec();
}