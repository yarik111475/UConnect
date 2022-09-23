#include <QScreen>
#include <QJsonArray>
#include <QApplication>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>

#include "TaskPlugin.h"
#include "win/WinUtils.h"

QString TaskPlugin::get_orientation(Qt::ScreenOrientation orientation)
{
    if(orientation_map_.empty()){
        orientation_map_.emplace(0x00, "PrimaryOrientation");
        orientation_map_.emplace(0x01, "PortraitOrientation");
        orientation_map_.emplace(0x02, "LandscapeOrientation");
        orientation_map_.emplace(0x04, "InvertedPortraitOrientation");
        orientation_map_.emplace(0x08, "InvertedLandscapeOrientation");
    }
    auto found=orientation_map_.find(orientation);
    if(found!=orientation_map_.end()){
        return found->second;
    }
    return "NotDefined";
}

QJsonArray TaskPlugin::get_displays_win()
{
    QJsonArray displays_array_json=WinUtils::displays_win();
    return displays_array_json;
}

QJsonArray TaskPlugin::get_displays_common()
{
    QJsonArray displays_array_json {};

    QList<QScreen*> screen_list = QApplication::screens();
    for(const auto* screen: screen_list){
        QJsonObject display_json;
        display_json.insert("manufacturer", screen->manufacturer());
        display_json.insert("model", screen->model());
        display_json.insert("serial", screen->serialNumber());
        display_json.insert("name", screen->name());
        display_json.insert("width", screen->availableSize().width());
        display_json.insert("height", screen->availableSize().height());
        /*display_json.insert("ppi", screen->physicalDotsPerInch());*/
        display_json.insert("orientation", get_orientation(screen->orientation()));
        display_json.insert("primary", (QApplication::primaryScreen()==screen) ? true : false);
        displays_array_json.append(display_json);
    }
    return displays_array_json;
}

TaskPlugin::TaskPlugin()
{
}

int TaskPlugin::pluginVersion() const
{
    return 0;
}

void TaskPlugin::handle_impl(const std::string &incoming_string, std::string &outcoming_string)
{
    QJsonArray displays_array_json {};

#ifdef Q_OS_WINDOWS
    displays_array_json=get_displays_win();
#endif
#ifdef Q_OS_UNIX
    displays_array_json=get_displays_common();
#endif
    outcoming_string=QJsonDocument(displays_array_json).toJson().toStdString();
}
