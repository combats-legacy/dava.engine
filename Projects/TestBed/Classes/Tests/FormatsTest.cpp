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


#include "Tests/FormatsTest.h"

using namespace DAVA;

namespace FormatsTestInternal
{
void UpdateSupportState(const PixelFormatDescriptor& descriptor, UIStaticText* formatValue)
{
    if (descriptor.isHardwareSupported)
    {
        formatValue->SetText(L"supported");
        formatValue->SetTextColor({ 0, 1.0f, 0, 1.f });
    }
    else
    {
        formatValue->SetText(L"unsupported");
        formatValue->SetTextColor({ 1.f, 0, 0, 1.f });
    }
}

void UpdateFormatInfo(PixelFormat format, UIStaticText* formatName, UIStaticText* formatValue)
{
    if (format == FORMAT_INVALID)
    {
        formatName->SetText(L"Invalid");
        formatValue->SetText(L"Ignore. For developers only");
        formatValue->SetTextColor({ 1.0f, 0.5f, 0.f, 1.0f });
        return;
    }

    if (format == FORMAT_REMOVED_DXT_1N)
    {
        formatName->SetText(L"Removed: DXT_1N");
        formatValue->SetText(L"unsupported");
        formatValue->SetTextColor({ 1.0f, 0.5f, 0.f, 1.0f });
        return;
    }

    const PixelFormatDescriptor& descriptor = PixelFormatDescriptor::GetPixelFormatDescriptor(format);
    formatName->SetText(StringToWString(descriptor.name.c_str()));

    if (format == FORMAT_DXT1A)
    {
        UpdateSupportState(PixelFormatDescriptor::GetPixelFormatDescriptor(FORMAT_DXT1), formatValue);
    }
    else if (format == FORMAT_DXT5NM)
    {
        UpdateSupportState(PixelFormatDescriptor::GetPixelFormatDescriptor(FORMAT_DXT5), formatValue);
    }
    else
    {
        UpdateSupportState(descriptor, formatValue);
    }
}
}

FormatsTest::FormatsTest()
    : BaseScreen("Supported texture formats test")
{
}

void FormatsTest::LoadResources()
{
    BaseScreen::LoadResources();

    DVASSERT(formatsGrid == nullptr);
    const Size2i screenSize = VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize();
    formatsGrid = new UIList(Rect(0, 0, static_cast<DAVA::float32>(screenSize.dx), static_cast<DAVA::float32>(screenSize.dy - 30.f)), UIList::ORIENTATION_VERTICAL);
    formatsGrid->SetDelegate(this);
    AddControl(formatsGrid);
}

void FormatsTest::UnloadResources()
{
    BaseScreen::UnloadResources();
    RemoveAllControls();

    SafeRelease(formatsGrid);
}

float32 FormatsTest::CellHeight(UIList* list, int32 index)
{
    return 30.f;
}

int32 FormatsTest::ElementsCount(UIList* list)
{
    return FORMAT_PVR2_2; //FORMAT_COUNT in future
}

UIListCell* FormatsTest::CellAtIndex(UIList* list, int32 index)
{
    UIStaticText* formatName = nullptr;
    UIStaticText* formatValue = nullptr;

    static const String cellName = "TestButtonCell";
    UIListCell* cell = list->GetReusableCell(cellName); //try to get cell from the reusable cells store
    if (!cell)
    { //if cell of requested type isn't find in the store create new cell
        ScopedPtr<Font> font(FTFont::Create("~res:/Fonts/korinna.ttf"));
        DVASSERT(font);
        font->SetSize(16.0f);

        const float32 cellWidth = list->GetSize().x;
        const float32 cellHeight = CellHeight(list, index);
        cell = new UIListCell(Rect(0.f, 0.f, cellWidth, cellHeight), cellName);
        cell->SetDebugDraw(true, false);

        const float32 formatNameWidth = Min(cellWidth / 2.0f, 200.0f);
        formatName = new UIStaticText(Rect(0., 0., formatNameWidth, cellHeight));
        formatName->SetFont(font);
        formatName->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
        formatName->SetFittingOption(TextBlock::FITTING_REDUCE);
        cell->AddControl(formatName);

        const float32 formatValueWidth = cellWidth - formatNameWidth;
        formatValue = new UIStaticText(Rect(formatNameWidth, 0., formatValueWidth, cellHeight));
        formatValue->SetFont(font);
        formatValue->SetTextAlign(ALIGN_LEFT | ALIGN_VCENTER);
        cell->AddControl(formatValue);
    }

    FormatsTestInternal::UpdateFormatInfo(static_cast<PixelFormat>(index), formatName, formatValue);
    return cell;
}

void FormatsTest::OnCellSelected(UIList* forList, UIListCell* selectedCell)
{
}
