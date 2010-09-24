#include "conversion/conversiontask.h"

ConversionTask::ConversionTask(IConversionService *service, QObject *parent) :
    QObject(parent), mService(service), mAborted(false)
{
}
