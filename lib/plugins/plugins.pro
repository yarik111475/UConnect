TEMPLATE = subdirs

#include($$PWD/ITaskPlugin/ITaskPlugin.pri)

SUBDIRS += \
    ITaskPlugin \
    TaskBaseSystemAudit \
    TaskBaseSystemMonitoring \
    TaskCheckAlive \
    TaskCpuDeviceAudit \
    TaskDisplayDeviceAudit \
    TaskDownloadFile \
    TaskDriveDeviceAudit \
    TaskExecuteCommand \
    TaskGpuDeviceAudit \
    TaskInitRemoteDesktopConnection \
    TaskLocalNetworkAudit \
    TaskNetworkDeviceAudit \
    TaskProcessMonitoring \
    TaskScreenshot \
    TaskServiceAudit \
    TaskSnmpMonitoring \
    TaskSoftAudit \
    TaskUpdate \
    TaskUpdateAgent \
    TaskUpdateClientCert \
    TaskUploadFile \
    TaskVmHyperVSystemAudit \
    TaskVmHyperVSystemMonitoring \
    TaskVmWareSystemAudit \
    TaskVmWareSystemMonitoring

CONFIG += ordered
