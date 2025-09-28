/*****************************************************************************
 * Copyright (C) 2021 VLC authors and VideoLAN
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * ( at your option ) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/
#ifndef ControlbarProfile_HPP
#define ControlbarProfile_HPP

#include <QObject>
#include <QMap>
#include <QVector>
#include <array>

Q_MOC_INCLUDE( "player/player_controlbar_model.hpp" )

class PlayerControlbarModel;

class ControlbarProfile : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool dirty READ dirty RESET resetDirty NOTIFY dirtyChanged FINAL)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)

    friend class ControlbarProfileModel;

public:
    explicit ControlbarProfile(QObject *parent = nullptr);

    PlayerControlbarModel* newModel(int identifier);
    Q_INVOKABLE PlayerControlbarModel* getModel(int identifier) const;

    void setModelData(int identifier, const std::array<QVector<int>, 3>& data);
    std::array<QVector<int>, 3> getModelData(int identifier) const;

    void deleteModel(int identifier);

    bool dirty() const;
    QString name() const;

    bool operator==(const ControlbarProfile& model) const;
    bool operator!=(const ControlbarProfile& model) const {
        return !(operator==(model));
    }

public slots:
    void resetDirty();
    void setName(const QString& name);

private:
    // m_dirty indicates the count of PlayerControlbarModel
    // residing in m_models which has the dirty property
    // set true.
    int m_dirty = 0;

    QString m_name = QStringLiteral("N/A");
    bool m_pauseControlListGeneration = false;

    // According to benchmarks, QMap performs better than
    // QHash when item count is less than 32.
    // Assuming model (player) count to stay below that,
    // QMap is used here.
    QMap<int, PlayerControlbarModel *> m_models;

    struct Configuration {
        int identifier;
        std::array<QVector<int>, 3> data;
    };

private:
    void injectModel(const QVector<Configuration>& modelData);

private slots:
    void generateLinearControlList();

signals:
    void dirtyChanged(bool dirty);
    void nameChanged(QString name);

    void controlListChanged(const QVector<int>& linearControlList);
};

#endif // ControlbarProfile_HPP
