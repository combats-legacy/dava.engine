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


#ifndef __RESOURCEEDITORQT__MODIFICATIONWIDGET__
#define __RESOURCEEDITORQT__MODIFICATIONWIDGET__

#include <QWidget>
#include <QAbstractSpinBox>
#include <QLabel>

#include "Scene/SceneEditor2.h"
#include "Scene/SceneSignals.h"

class DAVAFloat32SpinBox;

class ModificationWidget
: public QWidget
{
    Q_OBJECT

public:
    enum PivotMode : DAVA::uint32
    {
        PivotAbsolute,
        PivotRelative,
    };

    explicit ModificationWidget(QWidget* parent = nullptr);

    void SetPivotMode(PivotMode pivotMode);
    void SetTransformType(Selectable::TransformType modifMode);

    void ReloadValues();

public slots:
    void OnSnapToLandscapeChanged();

private slots:
    void OnSceneActivated(SceneEditor2* scene);
    void OnSceneDeactivated(SceneEditor2* scene);
    void OnSceneSelectionChanged(SceneEditor2* scene, const SelectableGroup* selected, const SelectableGroup* deselected);
    void OnSceneCommand(SceneEditor2* scene, const Command2* command, bool redo);

    void OnXChanged();
    void OnYChanged();
    void OnZChanged();

private:
    void ApplyValues(ST_Axis axis);

private:
    QLabel* xLabel = nullptr;
    QLabel* yLabel = nullptr;
    QLabel* zLabel = nullptr;
    DAVAFloat32SpinBox* xAxisModify = nullptr;
    DAVAFloat32SpinBox* yAxisModify = nullptr;
    DAVAFloat32SpinBox* zAxisModify = nullptr;
    SceneEditor2* curScene = nullptr;
    PivotMode pivotMode = PivotMode::PivotAbsolute;
    Selectable::TransformType modifMode = Selectable::TransformType::Disabled;
    bool groupMode = false;
};

class DAVAFloat32SpinBox
: public QAbstractSpinBox
{
    Q_OBJECT

public:
    explicit DAVAFloat32SpinBox(QWidget* parent = nullptr);
    virtual ~DAVAFloat32SpinBox();

    void showButtons(bool show);

    DAVA::float32 value() const;
    void setValue(DAVA::float32 val);
    void stepBy(int steps) override;

signals:
    void valueEdited();
    void valueChanged();

public slots:
    void clear() override;

protected slots:
    void textEditingFinished();

protected:
    bool eventFilter(QObject* object, QEvent* event) override;

    DAVA::float32 originalValue;
    QString originalString;

    int precision;
    bool hasButtons;
    bool cleared;

    void keyPressEvent(QKeyEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    StepEnabled stepEnabled() const override;
};

#endif /* defined(__RESOURCEEDITORQT__MODIFICATIONWIDGET__) */
