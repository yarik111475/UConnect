#include <QCoreApplication>

#include "service/RemoteService.h"
#include "service/LocalService.h"


int main(int argc, char *argv[])
{
#ifdef DEBUG
    LocalService local_service(argc,argv);
    local_service.init_addresses();
    local_service.start();
    return local_service.exec();
#endif

#ifdef RELEASE
    RemoteService remote_service(argc,argv);
    remote_service.init_addresses();
    remote_service.exec();
#endif
}
