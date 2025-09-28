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
#ifndef CONTROLBARPROFILEMODEL_H
#define CONTROLBARPROFILEMODEL_H

#include <QAbstractListModel>
#include <QSettings>
#include <array>

#include "controlbar_profile.hpp"


class ControlbarProfileModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int selectedProfile READ selectedProfile WRITE setSelectedProfile NOTIFY selectedProfileChanged FINAL)
    Q_PROPERTY(ControlbarProfile* currentModel READ currentModel NOTIFY selectedProfileChanged FINAL)

    Q_PROPERTY(int count READ rowCount NOTIFY countChanged FINAL)

public:
    explicit ControlbarProfileModel(QSettings* settings, QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override;

    enum class Style
    {
        EMPTY_STYLE,
        DEFAULT_STYLE,
        MINIMALIST_STYLE,
        ONE_LINER_STYLE,
        SIMPLEST_STYLE,
        CLASSIC_STYLE
    };

    // Editable:
    Q_INVOKABLE bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::DisplayRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // Add data:
    Q_INVOKABLE bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    // Remove data:
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

public:
    enum Roles {
        MODEL_ROLE = Qt::UserRole,
    };

    int selectedProfile() const;
    ControlbarProfile* currentModel() const;

    ControlbarProfile* cloneProfile(const ControlbarProfile* profile);
    Q_INVOKABLE void cloneSelectedProfile(const QString& newProfileName);

    Q_INVOKABLE ControlbarProfile* getProfile(int index) const;

    Q_INVOKABLE ControlbarProfile* newProfile(const QString& name);
    ControlbarProfile* newProfile();

    Q_INVOKABLE void deleteSelectedProfile();

    // This is a static method, caller takes the ownership
    // of the profile:
    static ControlbarProfile* generateProfileFromStyle(const Style style);

    // ControlbarProfileModel takes the ownership:
    void insertProfile(std::unique_ptr<ControlbarProfile> profile, bool select);

public slots:
    void save(bool clearDirty = true) const;
    bool reload();

    bool setSelectedProfile(int selectedProfile);

    std::optional<int> findModel(const ControlbarProfile* profile) const;

signals:
    void countChanged();
    void selectedProfileChanged();

    void selectedProfileControlListChanged(const QVector<int>& linearControlList);

private:
    QVector<ControlbarProfile *> m_profiles;

    int m_selectedProfile = -1;

    struct Profile {
        const Style id;
        const char* name;
        QVector<ControlbarProfile::Configuration> modelData;
    };

    static const QVector<Profile> m_defaults;

private:
    void insertDefaults();

    QString generateUniqueName(const QString& name);

protected:
    QSettings* m_settings = nullptr;
};

#endif // CONTROLBARPROFILEMODEL_H
