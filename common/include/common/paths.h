#ifndef PATHS_H
#define PATHS_H

#include <QObject>
#include <QDir>
#include <QString>
#include <QMetaType>

#include "common/global.h"

class PathsData;

namespace EvilTemple {

/**
  This class gives easy access to the file system paths to important directories.
  */
class COMMON_EXPORT Paths : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString userDataPath READ userDataPath)
    Q_PROPERTY(QString generatedDataPath READ generatedDataPath)
    Q_PROPERTY(QString installationPath READ installationPath)
public:
    /**
      Constructs the path object and runs the path autodetection if disableAutodetection is not true.

      Please note that QApplication/QCoreApplication needs to be instantiated somewhere before this
      class is instantiated, since it is used to determine from where the application is run.

      @see autodetectPaths()
      */
    explicit Paths(bool disableAutodetection = false, QObject *parent = 0);
    ~Paths();

    /**
      Runs the path autodetection and sets all paths and directories accordingly.

      This method is called by the constructor if not disabled explicitly.

      The auto-detection process will first check if there are command-line arguments present
      that override the paths. The following command line options are recognized:
      <dl>
        <dt>-installPath &lt;path&gt;
        <dd>Sets the installation path to the given path.

        <dt>-userDataPath &lt;path&gt;
        <dd>Sets the user data path to the given path.

        <dt>-generatedDataPath &lt;path&gt;
        <dd>Sets the generated data path to the given path.
      </dl>

      Please note that directories given on the command-line will not be created automatically and the
      application will malfunction if the paths don't exist.
      */
    void autodetectPaths();

    /**
      Gets directory for user data. This directory is intended to store the following information:
      - Save games
      - Screenshots
      - Configuration data
      - Character files
      - Other valuable per-user data

      Usually this will be a subfolder of the user's document folder.
      */
    const QDir &userDataDir() const;

    /**
      Returns the path to the user data directory.
      The path is guaranteed to end with a directory separator.

      @see userDataDir()
      */
    const QString &userDataPath() const;

    /**
      Manually overrides the user data directory and changes the user data path accordingly.

      @see userDataDir()
      @see userDataPath()
      */
    void setUserDataDir(const QDir &dir);

    /**
      Gets the directory for generated files. This directory is intended to store files that are generated, but
      of no importance to the user (no backup is necessary).

      Usually this will be a subfolder of either the user's home directory, or
      rather the local AppData folder (on Windows).
      */
    const QDir &generatedDataDir() const;

    /**
      Returns the path to the user data directory.

      The path is guaranteed to end with a directory separator.

      @see generatedDataPath()
      */
    const QString &generatedDataPath() const;

    /**
      Manually overrides the directory for generated application data and changes the corresponding path accordingly.
      @see generatedDataDir()
      @see generatedDataPath()
      */
    void setGeneratedDataDir(const QDir &dir);

    /**
      Returns the root directory in which the game is installed. Do not assume that this directory is writeable,
      most of the time it's not.

      This will usually be the parent directory of the directory containing the application executable file.
      */
    const QDir &installationDir() const;

    /**
      Returns the path to the program directory.

      The path is guaranteed to end with a directory separator.

      @see programDataDir()
      */
    const QString &installationPath() const;

    /**
      Manually overrides the installation directory. This will also change the
      installation path.

      @see installationDir()
      @see installationPath()
      */
    void setInstallationDir(const QDir &dir);

private:
    PathsData *d;
};

}

Q_DECLARE_METATYPE(EvilTemple::Paths*)

#endif // PATHS_H
