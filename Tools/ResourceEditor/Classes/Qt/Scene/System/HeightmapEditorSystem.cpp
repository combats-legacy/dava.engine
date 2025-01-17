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


#include "HeightmapEditorSystem.h"
#include "CollisionSystem.h"
#include "SelectionSystem.h"
#include "ModifSystem.h"
#include "Scene/SceneEditor2.h"
#include "Scene/SceneSignals.h"
#include "LandscapeEditorDrawSystem/HeightmapProxy.h"
#include "LandscapeEditorDrawSystem/LandscapeProxy.h"
#include "Commands2/HeightmapEditorCommands2.h"
#include "Main/QtUtils.h"
#include "HoodSystem.h"

#include "Render/Image/ImageConvert.h"

#include <QApplication>

HeightmapEditorSystem::HeightmapEditorSystem(DAVA::Scene* scene)
    : LandscapeEditorSystem(scene, "~res:/LandscapeEditor/Tools/cursor/cursor.tex")
    , copyPasteFrom(-1.f, -1.f)
    , copyPasteTo(-1.f, -1.f)
{
    activeDrawingType = drawingType;
}

HeightmapEditorSystem::~HeightmapEditorSystem()
{
    SafeRelease(curToolImage);
    SafeRelease(squareTexture);
}

LandscapeEditorDrawSystem::eErrorType HeightmapEditorSystem::EnableLandscapeEditing()
{
    if (enabled)
    {
        return LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
    }

    LandscapeEditorDrawSystem::eErrorType canBeEnabledError = IsCanBeEnabled();
    if (canBeEnabledError != LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
    {
        return canBeEnabledError;
    }

    LandscapeEditorDrawSystem::eErrorType enableCustomDrawError = drawSystem->EnableCustomDraw();
    if (enableCustomDrawError != LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
    {
        return enableCustomDrawError;
    }

    selectionSystem->SetLocked(true);
    modifSystem->SetLocked(true);

    landscapeSize = drawSystem->GetHeightmapProxy()->Size();
    copyPasteFrom = DAVA::Vector2(-1.f, -1.f);

    drawSystem->EnableCursor();
    drawSystem->SetCursorTexture(cursorTexture);
    drawSystem->GetBaseLandscape()->SetUpdatable(true);

    SetBrushSize(curToolSize);

    enabled = true;
    return LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

bool HeightmapEditorSystem::DisableLandscapeEdititing()
{
    if (!enabled)
    {
        return true;
    }

    FinishEditing();

    selectionSystem->SetLocked(false);
    modifSystem->SetLocked(false);

    drawSystem->DisableCursor();
    drawSystem->DisableCustomDraw();

    enabled = false;
    return !enabled;
}

void HeightmapEditorSystem::Process(DAVA::float32 timeElapsed)
{
    if (!IsLandscapeEditingEnabled())
    {
        return;
    }

    if (editingIsEnabled && isIntersectsLandscape)
    {
        UpdateBrushTool(timeElapsed);
    }
}

void HeightmapEditorSystem::Input(DAVA::UIEvent* event)
{
    if (!IsLandscapeEditingEnabled())
    {
        return;
    }

    UpdateCursorPosition();

    if (event->mouseButton == DAVA::UIEvent::MouseButton::LEFT)
    {
        DAVA::Vector3 point;

        switch (event->phase)
        {
        case DAVA::UIEvent::Phase::BEGAN:
            if (drawingType == HEIGHTMAP_DRAW_ABSOLUTE_DROPPER ||
                drawingType == HEIGHTMAP_DROPPER)
            {
                curHeight = drawSystem->GetHeightAtHeightmapPoint(GetHeightmapPositionFromCursor());

                SceneSignals::Instance()->EmitDropperHeightChanged(dynamic_cast<SceneEditor2*>(GetScene()), curHeight);
            }

            if (isIntersectsLandscape)
            {
                if (drawingType == HEIGHTMAP_COPY_PASTE)
                {
                    DAVA::int32 curKeyModifiers = QApplication::keyboardModifiers();
                    if (curKeyModifiers & Qt::AltModifier)
                    {
                        copyPasteFrom = GetHeightmapPositionFromCursor();
                        copyPasteTo = DAVA::Vector2(-1.f, -1.f);
                        return;
                    }
                    else
                    {
                        if (copyPasteFrom == DAVA::Vector2(-1.f, -1.f))
                        {
                            return;
                        }
                        copyPasteTo = GetHeightmapPositionFromCursor();
                        StoreOriginalHeightmap();
                    }
                }
                else
                {
                    if (drawingType != HEIGHTMAP_DROPPER)
                    {
                        StoreOriginalHeightmap();
                    }
                }

                editingIsEnabled = true;
            }

            activeDrawingType = drawingType;
            break;

        case DAVA::UIEvent::Phase::DRAG:
            break;

        case DAVA::UIEvent::Phase::ENDED:
            FinishEditing();
            break;

        default:
            break;
        }
    }
}

void HeightmapEditorSystem::FinishEditing()
{
    if (editingIsEnabled)
    {
        if (drawingType != HEIGHTMAP_DROPPER)
        {
            CreateHeightmapUndo();
        }
        editingIsEnabled = false;
    }
}

void HeightmapEditorSystem::UpdateToolImage()
{
    if (!toolImagePath.IsEmpty())
    {
        SafeRelease(curToolImage);

        DAVA::Vector<DAVA::Image*> images;
        DAVA::ImageSystem::Instance()->Load(toolImagePath, images);
        if (images.size())
        {
            DVASSERT(images.size() == 1);
            DVASSERT(images[0]->GetPixelFormat() == DAVA::FORMAT_RGBA8888);

            curToolImage = DAVA::Image::Create(curToolSize, curToolSize, DAVA::FORMAT_RGBA8888);
            DAVA::ImageConvert::ResizeRGBA8Billinear(reinterpret_cast<DAVA::uint32*>(images[0]->data), images[0]->GetWidth(), images[0]->GetHeight(),
                                                     reinterpret_cast<DAVA::uint32*>(curToolImage->data), curToolSize, curToolSize);

            SafeRelease(images[0]);
        }
    }
}

void HeightmapEditorSystem::UpdateBrushTool(DAVA::float32 timeElapsed)
{
    if (!curToolImage)
    {
        DAVA::Logger::Error("Tool image is empty!");
        return;
    }

    EditorHeightmap* editorHeightmap = drawSystem->GetHeightmapProxy();

    DAVA::int32 scaleSize = curToolImage->GetWidth();
    DAVA::Vector2 pos = GetHeightmapPositionFromCursor() - DAVA::Vector2(static_cast<DAVA::float32>(scaleSize), static_cast<DAVA::float32>(scaleSize)) / 2.0f;
    {
        switch (activeDrawingType)
        {
        case HEIGHTMAP_DRAW_RELATIVE:
        {
            DAVA::float32 koef = (strength * timeElapsed);
            if (inverseDrawingEnabled)
            {
                koef = -koef;
            }

            if (IsKeyModificatorPressed(DAVA::Key::LALT))
            {
                koef = -koef;
            }

            editorHeightmap->DrawRelativeRGBA(curToolImage, static_cast<DAVA::int32>(pos.x), static_cast<DAVA::int32>(pos.y), scaleSize, scaleSize, koef);
            break;
        }

        case HEIGHTMAP_DRAW_AVERAGE:
        {
            DAVA::float32 koef = (averageStrength * timeElapsed) * 2.0f;
            editorHeightmap->DrawAverageRGBA(curToolImage, static_cast<DAVA::int32>(pos.x), static_cast<DAVA::int32>(pos.y), scaleSize, scaleSize, koef);
            break;
        }

        case HEIGHTMAP_DRAW_ABSOLUTE:
        case HEIGHTMAP_DRAW_ABSOLUTE_DROPPER:
        {
            DAVA::float32 maxHeight = drawSystem->GetLandscapeMaxHeight();
            DAVA::float32 height = curHeight / maxHeight * DAVA::Heightmap::MAX_VALUE;

            DAVA::float32 koef = (averageStrength * timeElapsed) * 2.0f;
            editorHeightmap->DrawAbsoluteRGBA(curToolImage, static_cast<DAVA::int32>(pos.x), static_cast<DAVA::int32>(pos.y), scaleSize, scaleSize, koef, height);
            break;
        }

        case HEIGHTMAP_DROPPER:
        {
            DAVA::float32 curHeight = drawSystem->GetHeightAtHeightmapPoint(GetHeightmapPositionFromCursor());
            SceneSignals::Instance()->EmitDropperHeightChanged(dynamic_cast<SceneEditor2*>(GetScene()), curHeight);
            return;
        }

        case HEIGHTMAP_COPY_PASTE:
        {
            if (copyPasteFrom == DAVA::Vector2(-1.f, -1.f) || copyPasteTo == DAVA::Vector2(-1.f, -1.f))
            {
                return;
            }

            DAVA::Vector2 posTo = pos;

            DAVA::Vector2 deltaPos = GetHeightmapPositionFromCursor() - copyPasteTo;
            DAVA::Vector2 posFrom = copyPasteFrom + deltaPos - DAVA::Vector2(static_cast<DAVA::float32>(scaleSize), static_cast<DAVA::float32>(scaleSize)) / 2.f;

            DAVA::float32 koef = (averageStrength * timeElapsed) * 2.0f;

            editorHeightmap->DrawCopypasteRGBA(curToolImage, posFrom, posTo, scaleSize, scaleSize, koef);

            break;
        }

        default:
            DAVA::Logger::Error("Invalid drawing type!");
            return;
        }

        DAVA::Rect rect(pos.x, pos.y, static_cast<DAVA::float32>(scaleSize), static_cast<DAVA::float32>(scaleSize));
        drawSystem->GetHeightmapProxy()->UpdateRect(rect);
        AddRectToAccumulator(heightmapUpdatedRect, rect);
    }
}

void HeightmapEditorSystem::ResetAccumulatorRect(DAVA::Rect& accumulator)
{
    DAVA::float32 inf = std::numeric_limits<DAVA::float32>::infinity();
    accumulator = DAVA::Rect(inf, inf, -inf, -inf);
}

void HeightmapEditorSystem::AddRectToAccumulator(DAVA::Rect& accumulator, const DAVA::Rect& rect)
{
    accumulator = accumulator.Combine(rect);
}

DAVA::Rect HeightmapEditorSystem::GetHeightmapUpdatedRect()
{
    DAVA::Rect r = heightmapUpdatedRect;
    drawSystem->ClampToHeightmap(r);
    return r;
}

void HeightmapEditorSystem::StoreOriginalHeightmap()
{
    EditorHeightmap* editorHeightmap = drawSystem->GetHeightmapProxy();

    DVASSERT(originalHeightmap == NULL);
    originalHeightmap = editorHeightmap->Clone(NULL);
    ResetAccumulatorRect(heightmapUpdatedRect);
}

void HeightmapEditorSystem::CreateHeightmapUndo()
{
    SceneEditor2* scene = dynamic_cast<SceneEditor2*>(GetScene());
    DVASSERT(scene);
    scene->Exec(Command2::Create<ModifyHeightmapCommand>(drawSystem->GetHeightmapProxy(), originalHeightmap, GetHeightmapUpdatedRect()));
    SafeRelease(originalHeightmap);
}

void HeightmapEditorSystem::SetBrushSize(DAVA::int32 brushSize)
{
    if (brushSize > 0)
    {
        curToolSize = brushSize;
        cursorSize = static_cast<DAVA::float32>(brushSize) / landscapeSize;
        drawSystem->SetCursorSize(cursorSize);

        UpdateToolImage();
    }
}

void HeightmapEditorSystem::SetStrength(DAVA::float32 strength)
{
    DAVA::float32 s = DAVA::Abs(strength);
    this->strength = s;

    inverseDrawingEnabled = false;
    if (strength < 0.f)
    {
        inverseDrawingEnabled = true;
    }
}

void HeightmapEditorSystem::SetAverageStrength(DAVA::float32 averageStrength)
{
    if (averageStrength >= 0)
    {
        this->averageStrength = averageStrength;
    }
}

void HeightmapEditorSystem::SetToolImage(const DAVA::FilePath& toolImagePath, DAVA::int32 index)
{
    this->toolImagePath = toolImagePath;
    this->toolImageIndex = index;
    UpdateToolImage();
}

void HeightmapEditorSystem::SetDrawingType(eHeightmapDrawType type)
{
    copyPasteFrom = DAVA::Vector2(-1.f, -1.f);
    drawingType = type;
}

DAVA::int32 HeightmapEditorSystem::GetBrushSize()
{
    return curToolSize;
}

DAVA::float32 HeightmapEditorSystem::GetStrength()
{
    DAVA::float32 s = strength;
    if (inverseDrawingEnabled)
    {
        s = -s;
    }

    return s;
}

DAVA::float32 HeightmapEditorSystem::GetAverageStrength()
{
    return averageStrength;
}

DAVA::int32 HeightmapEditorSystem::GetToolImageIndex()
{
    return toolImageIndex;
}

HeightmapEditorSystem::eHeightmapDrawType HeightmapEditorSystem::GetDrawingType()
{
    return drawingType;
}

void HeightmapEditorSystem::SetDropperHeight(DAVA::float32 height)
{
    DAVA::float32 maxHeight = drawSystem->GetLandscapeMaxHeight();

    if (height >= 0 && height <= maxHeight)
    {
        curHeight = height;
        SceneSignals::Instance()->EmitDropperHeightChanged(static_cast<SceneEditor2*>(GetScene()), curHeight);
    }
}

DAVA::float32 HeightmapEditorSystem::GetDropperHeight()
{
    return curHeight;
}

DAVA::Vector2 HeightmapEditorSystem::GetHeightmapPositionFromCursor() const
{
    return drawSystem->GetHeightmapProxy()->Size() * DAVA::Vector2(cursorPosition.x, 1.f - cursorPosition.y);
}
