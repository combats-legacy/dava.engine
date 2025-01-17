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


#ifndef __MATERIALS_ITEM_H__
#define __MATERIALS_ITEM_H__

#include "Render/Material/NMaterial.h"

#include <QObject>
#include <QStandardItem>

class MaterialModel;
class MaterialItem
: public QObject
  ,
  public QStandardItem
{
    Q_OBJECT

public:
    enum MaterialFlag : DAVA::uint32
    {
        IS_MARK_FOR_DELETE = 0x1,
        IS_PART_OF_SELECTION = 0x2,
    };

    MaterialItem(DAVA::NMaterial* material, bool dragEnabled, bool dropEnabled);
    virtual ~MaterialItem();

    QVariant data(int role = Qt::UserRole + 1) const;
    DAVA::NMaterial* GetMaterial() const;

    void SetFlag(MaterialFlag flag, bool set);
    bool GetFlag(MaterialFlag flag) const;

    void SetLodIndex(DAVA::int32 index);
    int GetLodIndex() const;

    void SetSwitchIndex(DAVA::int32 index);
    int GetSwitchIndex() const;

    void requestPreview();

private slots:
    void onThumbnailReady(const QList<QImage>& images, QVariant userData);

private:
    DAVA::NMaterial* material;
    DAVA::uint32 curFlag = 0;
    DAVA::int32 lodIndex = -1;
    DAVA::int32 switchIndex = -1;
    bool isPreviewRequested = false;
};


#endif // __MATERIALS_ITEM_H__
