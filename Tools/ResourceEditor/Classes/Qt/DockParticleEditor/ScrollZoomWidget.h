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


#ifndef TIMELINE_WIDGET_BASE_H
#define TIMELINE_WIDGET_BASE_H

#include <DAVAEngine.h>

#include <QWidget>
#include <QString>
#include <QDialog>
#include <QScrollBar>
#include <QSlider>
#include <QPainter>

#define SCALE_WIDTH 25

#define GRAPH_HEIGHT 150
#define GRAPH_OFFSET_STEP 10

#define MINIMUM_DISPLAYED_TIME 0.02f
#define ZOOM_STEP 0.1f
#define UI_RECTANGLE_OFFSET 1.5

#define SCROLL_BAR_HEIGHT 12

#define MIN_ZOOM 1.0f
#define MAX_ZOOM 10.0f
#define ZOOM_SLIDER_LENGTH 40

#ifdef __DAVAENGINE_WIN32__
#define SLIDER_HEIGHT_EXPAND 0
#else
#define SLIDER_HEIGHT_EXPAND 5
#endif

class ScrollZoomWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ScrollZoomWidget(QWidget* parent = 0);
    ~ScrollZoomWidget();

    virtual void Init(DAVA::float32 minT, DAVA::float32 maxT);
    DAVA::float32 GetMinBoundary();
    DAVA::float32 GetMaxBoundary();

signals:
    void ValueChanged();

protected:
    void paintEvent(QPaintEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;

    void wheelEvent(QWheelEvent*) override;

    virtual void UpdateSizePolicy() = 0;
    virtual QRect GetGraphRect() const = 0;
    virtual QRect GetIncreaseRect() const = 0;
    virtual QRect GetScaleRect() const = 0;
    virtual QRect GetDecreaseRect() const = 0;
    virtual QRect GetSliderRect() const = 0;

    virtual QRect GetScrollBarRect() const;

    void UpdateScrollBarPosition();
    void UpdateScrollBarSlider();

    void UpdateSliderPosition();
    void UpdateZoomSlider();

    QString float2QString(DAVA::float32 value) const;

    DAVA::int32 GetIntValue(DAVA::float32 value) const;

    void PerformZoom(DAVA::float32 newScale, bool moveScroll = true);

    void PerformOffset(DAVA::float32 value, bool moveScroll = true);

    enum ePositionRelativelyToDrawRect
    {
        POSITION_LEFT,
        POSITION_RIGHT,
        POSITION_INSIDE
    };
    ePositionRelativelyToDrawRect GetPointPositionFromDrawingRect(QPoint point) const;

protected slots:

    void HandleHorizontalScrollChanged(int value);
    void HandleZoomScrollChanged(int value);

protected:
    QPoint mouseStartPos;

    DAVA::float32 minValue;
    DAVA::float32 maxValue;
    DAVA::float32 minTime;
    DAVA::float32 maxTime;
    DAVA::float32 generalMinTime;
    DAVA::float32 generalMaxTime;
    DAVA::float32 minValueLimit;
    DAVA::float32 maxValueLimit;

    enum eGridStyle
    {
        GRID_STYLE_ALL_POSITION,
        GRID_STYLE_LIMITS
    };
    eGridStyle gridStyle;

    QBrush backgroundBrush;
    DAVA::float32 scale;
    DAVA::float32 initialTimeInterval;

    QScrollBar* horizontalScrollBar;
    QSlider* zoomSlider;
};


#endif // TIMELINE_WIDGET_BASE_H
