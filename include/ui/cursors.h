#ifndef CURSORS_H
#define CURSORS_H

#include <QObject>
#include <QCursor>

namespace EvilTemple {

    class VirtualFileSystem;
    class CursorsData;

    class Cursors : public QObject {

        Q_OBJECT
    public:
        explicit Cursors(VirtualFileSystem *vfs, QObject *parent);
        ~Cursors();       

        /**
          Cursor type.
        */
        enum Type {
             Normal = 0,
             NormalInvalid,
             Arrow,
             ArrowInvalid,
             AssignKey,
             DeleteFlag,
             Move,
             MoveGreen,
             MoveRed,
             MoveYellow,
             HaveKey,
             HotkeySelection,
             HotkeySelectionInvalid,
             Identify,
             Locked,
             GrabHandClosed,
             GrabHandOpen,
             PlaceFlag,
             SlideHorizontal,
             Attack,
             AttackInvalid,
             Talk,
             TalkInvalid,
             UsePotion,
             UsePotionInvalid,
             UseSkill,
             UseSkillInvalid,
             UseSpell,
             UseSpellInvalid,
             UseTeleport,
             UseWand,
             UseWandInvalid,
             ZoomCursor,
             Count
        };

        const QCursor &get(Type type) const;

    private:
        QScopedPointer<CursorsData> d_ptr;
        Q_DISABLE_COPY(Cursors)
    };

}

#endif // CURSORS_H
