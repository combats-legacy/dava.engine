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


#ifndef __RESOURCE_EDITOR_PARTICLEEMITTERPROPERTIESWIDGET_H__
#define __RESOURCE_EDITOR_PARTICLEEMITTERPROPERTIESWIDGET_H__

#include <QWidget>
#include <QVBoxLayout>
#include "GradientPickerWidget.h"
#include "TimeLineWidget.h"
#include "BaseParticleEditorContentWidget.h"
#include "Tools/EventFilterDoubleSpinBox/EventFilterDoubleSpinBox.h"

#include <QComboBox>
#include <QLabel>
#include <QSlider>
#include <QCheckBox>

class ParticleEmitterPropertiesWidget : public BaseParticleEditorContentWidget
{
    Q_OBJECT

public:
    explicit ParticleEmitterPropertiesWidget(QWidget* parent = nullptr);

    void Init(SceneEditor2* scene, DAVA::ParticleEffectComponent* effect, DAVA::ParticleEmitterInstance* emitter,
              bool updateMinimize, bool needUpdateTimeLimits = true);
    void Update();

    bool eventFilter(QObject* o, QEvent* e) override;

    void StoreVisualState(DAVA::KeyedArchive* visualStateProps) override;
    void RestoreVisualState(DAVA::KeyedArchive* visualStateProps) override;

    // Accessors to timelines.
    TimeLineWidget* GetEmitterRadiusTimeline()
    {
        return emitterRadius;
    };
    TimeLineWidget* GetEmitterAngleTimeline()
    {
        return emitterAngle;
    };
    TimeLineWidget* GetEmitterSizeTimeline()
    {
        return emitterSize;
    };
    TimeLineWidget* GetEmissionVectorTimeline()
    {
        return emitterEmissionVector;
    };

signals:
    void ValueChanged();

public slots:
    void OnValueChanged();
    void OnEmitterYamlPathChanged(const QString& newPath);
    void OnEmitterPositionChanged();
    void OnCommand(SceneEditor2* scene, const Command2* command, bool redo);

protected:
    void UpdateProperties();
    void UpdateTooltip();

private:
    QVBoxLayout* mainLayout = nullptr;
    QLineEdit* emitterNameLineEdit = nullptr;
    QLineEdit* originalEmitterYamlPath = nullptr;
    QLineEdit* emitterYamlPath = nullptr;
    QComboBox* emitterType = nullptr;
    EventFilterDoubleSpinBox* positionXSpinBox = nullptr;
    EventFilterDoubleSpinBox* positionYSpinBox = nullptr;
    EventFilterDoubleSpinBox* positionZSpinBox = nullptr;
    QCheckBox* shortEffectCheckBox = nullptr;

    TimeLineWidget* emitterEmissionRange = nullptr;
    TimeLineWidget* emitterEmissionVector = nullptr;
    TimeLineWidget* emitterRadius = nullptr;
    TimeLineWidget* emitterSize = nullptr;
    TimeLineWidget* emitterAngle = nullptr;
    EventFilterDoubleSpinBox* emitterLife = nullptr;
    GradientPickerWidget* emitterColorWidget = nullptr;

    bool blockSignals = false;
    bool updateMinimize = false;
    bool needUpdateTimeLimits = false;

    void InitWidget(QWidget* widget, bool connectWidget = true);
};

#endif // __RESOURCE_EDITOR_PARTICLEEMITTERPROPERTIESWIDGET_H__