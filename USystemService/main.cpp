#include <QCoreApplication>

#include "service/RemoteService.h"
#include "service/LocalService.h"


int main(int argc, char *argv[])
{
    RemoteService remote_service(argc,argv, "USystemService");
    remote_service.init_addresses();
    remote_service.exec();

    //LocalService local_service(argc,argv);
    //local_service.init_addresses();
    //local_service.start();
    //return local_service.exec();
}
