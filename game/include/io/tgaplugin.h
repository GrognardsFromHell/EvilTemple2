/*
 * TgaPlugin.h
 *
 *  Created on: 16.12.2009
 *      Author: Sebastian
 */

#ifndef TGAPLUGIN_H_
#define TGAPLUGIN_H_

#include <QImageIOPlugin>

namespace EvilTemple
{

class TgaPlugin : public QImageIOPlugin
{
	Q_OBJECT
public:
	TgaPlugin();
	virtual ~TgaPlugin();

	Capabilities capabilities(QIODevice *device, const QByteArray &format) const;
	QImageIOHandler *create(QIODevice *device, const QByteArray &format) const;
	QStringList keys() const;
};

class TgaHandler : public QImageIOHandler {
public:
	TgaHandler();

	bool canRead() const;
	bool read(QImage *image);
};

}

#endif /* TGAPLUGIN_H_ */
