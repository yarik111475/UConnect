#include <vector>
#include <QtCore>
#include <QtGlobal>

#include "task/Task.h"
#include "service/LocalService.h"

int main(int argc, char *argv[])
{
    qRegisterMetaType<std::vector<TaskItem>>();
    LocalService local_service(argc, argv);
    local_service.start();
    return local_service.exec();
#ifdef DEBUG

#endif

#ifdef RELEASE
    LocalService local_service(argc, argv);
    local_service.start();
    return local_service.exec();
#endif
}
