#include <string>
#include <iostream>

#include <QDir>
#include <QFile>
#include <QLibrary>
#include <QFileInfo>
#include <QApplication>

#include "ITaskPlugin.h"

using task_name_func_type=const char*(*)();
using create_plugin_func_type = ITaskPlugin*(*)();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString task_name {};
    QDir plugin_dir{QCoreApplication::applicationDirPath() + "/../plugins/"};
    QList<QFileInfo> entry_list=plugin_dir.entryInfoList(QStringList() << "*.dll" << "*.so", QDir::Files);
    for(const QFileInfo& entry: entry_list){
        QLibrary library(entry.absoluteFilePath());
        if(!library.load()){
            continue;
        }

        task_name_func_type plugin_name_func=(task_name_func_type)library.resolve("taskName");
        if(plugin_name_func){
            task_name=QString(plugin_name_func());
            if(!task_name.isEmpty()){
                if(task_name=="displayDeviceAudit"){
                    create_plugin_func_type create_plugin_func=(create_plugin_func_type)library.resolve("createPlugin");
                    if(create_plugin_func){

                        ITaskPlugin* plugin_ptr=create_plugin_func();
                        std::string incoming_string {};
                        std::string outcoming_string {};

                        QFile in("C:/tasks_json/" + task_name + ".json");
                        if(in.open(QIODevice::ReadOnly)){
                            incoming_string=in.readAll().toStdString();
                            plugin_ptr->handle(incoming_string, outcoming_string);
                            std::cout << outcoming_string << std::endl;
                        }

                    }
                }
            }
        }
    }
    return a.exec();
}
