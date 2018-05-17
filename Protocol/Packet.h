#ifndef _PACKET_
#define _PACKET_
#include <QObject>

class Packet : public QObject
{
    Q_OBJECT
private:
    virtual QByteArray specificDump() const = 0;
    virtual quint32 specificGetID() const = 0;
public:
    virtual ~Packet() {}
    // NVI
    const QByteArray dump() const;
}

#endif