#ifndef AUDIOENGINEGLOBAL_H
#define AUDIOENGINEGLOBAL_H

#include <QtCore/QtGlobal>

#if defined(AUDIOENGINE_LIBRARY)
#define AUDIOENGINE_EXPORT Q_DECL_EXPORT
#else
#define AUDIOENGINE_EXPORT Q_DECL_IMPORT
#endif

namespace EvilTemple {

/**
  Describes the category of a playing sound. This mostly effects which volume setting is used.
  */
enum SoundCategory {
    SoundCategory_Music = 0,
    SoundCategory_Effect, // Generic sound effect
    SoundCategory_Interface, // Interface sound effects like hovering/clicking a button
    SoundCategory_Ambience, // Ambient sound effects like the chirping of birds
    SoundCategory_Movie, // Movie soundtrack
    SoundCategory_Other,
    SoundCategory_Count, // Don't use this literal. Its only for faster mapping from this enum to values in an array
};

}

#endif // AUDIOENGINEGLOBAL_H
