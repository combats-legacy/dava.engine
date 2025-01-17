/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <QWidget>
#include <QScopedPointer>
#include <QMap>
#include <QEventLoop>
#include <QPointer>

namespace Ui
{
class ColorPicker;
};

#include "AbstractColorPicker.h"

class EyeDropper;

class ColorPicker
: public AbstractColorPicker
{
    Q_OBJECT

private:
    typedef QMap<QString, QPointer<AbstractColorPicker>> PickerMap;

public:
    explicit ColorPicker(QWidget* parent = 0);
    ~ColorPicker();

    void exec();

protected:
    void RegisterPicker(const QString& key, AbstractColorPicker* picker);
    void RegisterColorSpace(const QString& key, AbstractColorPicker* picker);

    void SetColorInternal(const QColor& c) override;

private slots:
    void OnChanging(const QColor& c);
    void OnChanged(const QColor& c);
    void OnDropperChanged(const QColor& c);

    void OnDropper();

private:
    void UpdateControls(const QColor& c, AbstractColorPicker* source = NULL);
    void ConnectPicker(AbstractColorPicker* picker);
    void closeEvent(QCloseEvent* e);

    QScopedPointer<Ui::ColorPicker> ui;
    QPointer<EyeDropper> dropper;
    PickerMap pickers;
    PickerMap colorSpaces;
    QEventLoop modalLoop;
};

#endif // COLORPICKER_H
