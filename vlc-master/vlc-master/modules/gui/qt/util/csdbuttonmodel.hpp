/*****************************************************************************
 * Copyright (C) 2022 the VideoLAN team
 *
 * Authors: Prince Gupta <guptaprince8832@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#ifndef CSDBUTTONCONTROLLER_HPP
#define CSDBUTTONCONTROLLER_HPP

#include <QObject>
#include <QRect>
#include <memory>

Q_MOC_INCLUDE("maininterface/mainctx.hpp")

class CSDButton : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ButtonType type READ type CONSTANT)
    Q_PROPERTY(bool showHovered READ showHovered WRITE setShowHovered NOTIFY showHoveredChanged)
    Q_PROPERTY(QRect rect READ rect WRITE setRect NOTIFY rectChanged)
    Q_PROPERTY(bool externalPressed READ externalPressed NOTIFY externalPressedChanged FINAL)

public:
    enum ButtonType
    {
        Minimize,
        MaximizeRestore,
        Close,
        SystemMenu,

        TypeCount
    };

    Q_ENUM(ButtonType)

    CSDButton(ButtonType type, QObject *parent);

    ButtonType type() const;

    // 'showHovered' is hint for UI to the show this
    // button as in 'hovered' state
    // used by implmentation incase custom event handling is required for CSD
    bool showHovered() const;
    void setShowHovered(bool newShowHovered);

    // 'rect' is location of the button in the UI
    // may be used by implementation to relay the information
    // to OS such as to show snaplay out menu on Windows 11
    const QRect &rect() const;
    void setRect(const QRect &newRect);

    bool externalPressed() const;
    void setExternalPressed();
    void unsetExternalPressed();
    void externalPress();
    void externalRelease();

public slots:
    // signals to perfrom action associated with button
    // actions are dependent on implmentation
    void click();
    void doubleClick();

signals:
    void showHoveredChanged();
    void rectChanged();
    void clicked();
    void doubleClicked();
    void externalPressedChanged();

private:
    const ButtonType m_type;
    bool m_showHovered = false;
    QRect m_rect;
    bool m_externalPressed = false;
};

class MainCtx;


class CSDButtonModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<CSDButton *> windowCSDButtons READ windowCSDButtons CONSTANT)

public:
    CSDButtonModel(MainCtx *mainCtx, QObject *parent = nullptr);

    // QList may cause crash on destruction (QTBUG-132126),
    // return by reference so that the QList rvalue is not
    // destructed:
    const QList<CSDButton *>& windowCSDButtons() const;

private slots:
    void minimizeButtonClicked();
    void maximizeRestoreButtonClicked();
    void closeButtonClicked();

private:
    MainCtx *m_mainCtx;

    // CSD window action buttons i.e minimize, maximize, close
    QList<CSDButton *> m_windowCSDButtons;
};


Q_DECLARE_METATYPE(CSDButton *)

#endif // CSDBUTTONCONTROLLER_HPP
