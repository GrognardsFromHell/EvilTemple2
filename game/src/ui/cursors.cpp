
#include <QCursor>
#include <QByteArray>
#include <QPixmap>

#include "ui/cursors.h"
#include "io/virtualfilesystem.h"

namespace EvilTemple {

    static const char *Filenames[Cursors::Count] = {
        "art/interface/cursors/MainCursor.tga",
        "art/interface/cursors/InvalidSelection.tga",
        "art/interface/cursors/arrow.tga",
        "art/interface/cursors/arrow_INVALID.tga",
        "art/interface/cursors/assignKeyCursor.tga",
        "art/interface/cursors/DeleteFlagCursor.tga",
        "art/interface/cursors/feet.tga",
        "art/interface/cursors/feet_green.tga",
        "art/interface/cursors/feet_red.tga",
        "art/interface/cursors/feet_yellow.tga",
        "art/interface/cursors/havekey.tga",
        "art/interface/cursors/hotkeyselection.tga",
        "art/interface/cursors/hotkeyselection_invalid.tga",
        "art/interface/cursors/identifycursor.tga",
        "art/interface/cursors/locked.tga",
        "art/interface/cursors/Map_GrabHand_Closed.tga",
        "art/interface/cursors/Map_GrabHand_Open.tga",
        "art/interface/cursors/PlaceFlagCursor.tga",
        "art/interface/cursors/SlidePortraits.tga",
        "art/interface/cursors/sword.tga",
        "art/interface/cursors/sword_invalid.tga",
        "art/interface/cursors/talk.tga",
        "art/interface/cursors/talk_invalid.tga",
        "art/interface/cursors/usepotion.tga",
        "art/interface/cursors/usepotion_invalid.tga",
        "art/interface/cursors/useskill.tga",
        "art/interface/cursors/useskill_invalid.tga",
        "art/interface/cursors/usespell.tga",
        "art/interface/cursors/usespell_invalid.tga",
        "art/interface/cursors/UseTeleportIcon.tga",
        "art/interface/cursors/wand.tga",
        "art/interface/cursors/wand_invalid.tga",
        "art/interface/cursors/zoomcursor.tga",
    };

    class CursorsData {
    public:
        VirtualFileSystem *vfs;
        QCursor cursors[Cursors::Count];

        /*
          Load Cursors
          */
        void load() {
            for (int i = 0; i < Cursors::Count; ++i) {
                QByteArray data = vfs->openFile(Filenames[i]);
                if (!data.isNull()) {
                    QPixmap pixmap;
                    if (pixmap.loadFromData(data, "tga")) {
                        cursors[i] = QCursor(pixmap, 0, 0);
                    }
                } else {
                    qWarning("Couldn't find cursor: %s", Filenames[i]);
                }
            }
        }
    };

    Cursors::Cursors(VirtualFileSystem *vfs, QObject *parent) :
        QObject(parent),
        d_ptr(new CursorsData)
    {
        d_ptr->vfs = vfs;
        d_ptr->load();
    }

    Cursors::~Cursors() {
    }

    const QCursor &Cursors::get(Type type) const {
        return d_ptr->cursors[type];
    }

}
