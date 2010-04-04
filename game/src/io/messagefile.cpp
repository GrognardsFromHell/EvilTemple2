
#include "io/messagefile.h"

namespace EvilTemple
{

    enum ParserState
    {
        WaitingForKey, // Everything until {
        WaitingForKeyComment, // Everything after // until newline, while waiting for key
        Key,
        WaitingForValue,
        WaitingForValueComment, // Everything after // until newline, while waiting for value
        Value
    };

    const QChar leftBracket('{');
    const QChar rightBracket('}');
    const QChar newline('\n');
    const QChar slash('/');

    QHash<quint32, QString> MessageFile::parse(const QByteArray &rawContent)
    {
        QString content = QString::fromLatin1(rawContent.data(), rawContent.size());

        QHash<quint32, QString> result;

        ParserState state = WaitingForKey;
        QChar previous;
        QString key, value;

        for (int i = 0; i < content.length(); ++i)
        {
            const QChar &c = content[i];

            switch (state)
            {
            case WaitingForKey:
                if (c == leftBracket)
                {
                    state = Key;
                }
                else if (c == slash && previous == slash)
                {
                    state = WaitingForKeyComment;
                }
                break;
            case WaitingForKeyComment:
                if (c == newline)
                {
                    state = WaitingForKey;
                }
                break;
            case Key:
                if (c == rightBracket)
                {
                    state = WaitingForValue;
                }
                else
                {
                    key.append(c);
                }
                break;
            case WaitingForValue:
                if (c == leftBracket)
                {
                    state = Value;
                }
                else if (c == slash && previous == slash)
                {
                    state = WaitingForValueComment;
                }
                break;
            case WaitingForValueComment:
                if (c == newline)
                {
                    state = WaitingForValue;
                }

                break;
            case Value:
                if (c == rightBracket)
                {
                    // Try parsing the key
                    bool ok;
                    int keyNumber = key.toInt(&ok);
                    if (ok)
                    {
                        result[keyNumber] = value;
                    }
                    else
                    {
                        qWarning("Invalid key for message: %s", qPrintable(key));
                    }

                    value.clear();
                    key.clear();
                    state = WaitingForKey;
                }
                else
                {
                    value.append(c);
                }
                break;
            }

            previous = c;
        }

        return result;
    }

}
