#include "EditorResourceManager.h"

#include "EditorSettings.h"

#include <QSettings>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QApplication>

QMap<QString, EditorGraphic *> EditorResourceManager::s_graphics;

EditorGraphic * EditorResourceManager::graphic(QString name)
{
    if( s_graphics.contains(name) ) {
        return s_graphics.value(name);
    } else {
        QDir dir(graphicsDir());
        QString filename = dir.absoluteFilePath(name);

        EditorGraphic * graphic = EditorGraphic::load(filename);
        if( graphic != NULL )
            s_graphics.insert(name, graphic);
        return graphic;
    }
}

bool EditorResourceManager::loadTextFile(QString filename, QList< QPair<QString, QString> > & out_list)
{
    QFile infile(filename);

    if( ! infile.open(QIODevice::ReadOnly | QIODevice::Text) ){
        qDebug() << "Error opening " << filename;
        return false;
    }

    out_list.clear();

    QTextStream in(&infile);
    while(! in.atEnd()) {
        QString line = in.readLine().trimmed();
        // handle backslash at the end
        while( line.mid(line.size()-1,1).compare("\\") == 0 ) {
            if( in.atEnd() ) {
                qDebug() << "Syntax error reading " << filename
                    << ": Backslash at EOF and no next line";
                infile.close();
                return false;
            }
            line.remove(line.size()-1, 1);
            line.append(in.readLine().trimmed());
        }

        if( line.size() == 0 || line.at(0) == '#' ) {
            // comment. ignore
            continue;
        } else {
            // break into name-value pairs
            QStringList pairs = line.split("=");
            if( pairs.size() < 2 ) {
                infile.close();
                qDebug() << "Syntax error reading " << filename;
                return false;
            }
            QString name = pairs.takeFirst().trimmed();
            QString value = pairs.join("").trimmed();

            out_list.append(QPair<QString, QString>(name, value));
        }
    }

    infile.close();
    return true;
}

QString EditorResourceManager::objectsDir()
{
    QDir dir(EditorSettings::dataFolder());
    return dir.absoluteFilePath("objects");
}

QString EditorResourceManager::localDataDir()
{
    // TODO: take into account data path so it works on linux
    QDir dir("data");
    return dir.absolutePath();
}

QString EditorResourceManager::workingDataDir()
{
    // TODO: take into account linux
    return QApplication::applicationDirPath();
}

QString EditorResourceManager::entitiesDir()
{
    QDir dir(EditorSettings::dataFolder());
    return dir.absoluteFilePath("entities");
}

QString EditorResourceManager::graphicsDir()
{
    QDir dir(EditorSettings::dataFolder());
    return dir.absoluteFilePath("graphics");
}

QString EditorResourceManager::mapsDir()
{
    QDir dir(EditorSettings::dataFolder());
    return dir.absoluteFilePath("maps");
}

QString EditorResourceManager::worldsDir()
{
    QDir dir(EditorSettings::dataFolder());
    return dir.absoluteFilePath("worlds");
}

QString EditorResourceManager::universesDir()
{
    QDir dir(EditorSettings::dataFolder());
    return dir.absoluteFilePath("universes");
}

QString EditorResourceManager::universeFile()
{
    return QDir(universesDir()).absoluteFilePath("main.universe");
}

QString EditorResourceManager::testUniverseFile()
{
    QString path = QDir(workingDataDir()).absoluteFilePath("playtest.universe");

    // if it doesn't exist, copy it from the real universe
    if (! QFileInfo(path).exists())
        QFile::copy(universeFile(), path);

    return path;
}

QString EditorResourceManager::newUniqueMapName(QString worldName)
{
    QDir maps(mapsDir());

    QString prefix;
    QString suffix = ".map";
    int digit = 1;
    if (worldName.isNull())
        prefix = "map_";
    else
        prefix = worldName + QString("-map_");

    while(QFileInfo(maps.absoluteFilePath(prefix + QString::number(digit) + suffix)).exists())
        ++digit;

    return prefix + QString::number(digit) + suffix;
}

bool EditorResourceManager::dataDirIsValid()
{
    QDir folder(EditorSettings::dataFolder());

    if (! folder.exists())
        return false;

    if (! QFileInfo(universesDir()).exists())
        return false;

    if (! QFileInfo(universeFile()).exists())
        return false;

    if (! QFileInfo(worldsDir()).exists())
        return false;

    if (! QFileInfo(entitiesDir()).exists())
        return false;

    if (! QFileInfo(graphicsDir()).exists())
        return false;

    if (! QFileInfo(mapsDir()).exists())
        return false;

    if (! QFileInfo(objectsDir()).exists())
        return false;

    return true;
}
