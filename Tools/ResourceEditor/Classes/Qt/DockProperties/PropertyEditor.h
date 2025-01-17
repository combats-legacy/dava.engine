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


#ifndef __QT_PROPERTY_WIDGET_H__
#define __QT_PROPERTY_WIDGET_H__

#include "PropertyEditorStateHelper.h"
#include "Tools/QtPosSaver/QtPosSaver.h"
#include "Tools/QtPropertyEditor/QtPropertyEditor.h"
#include "Scene/SceneSignals.h"

class LazyUpdater;
struct PropEditorUserData : public QtPropertyData::UserData
{
    enum PropertyType : DAVA::uint32
    {
        ORIGINAL,
        COPY
    };

    PropEditorUserData(PropertyType _type, QtPropertyData* _associatedData = NULL, bool _isFavorite = false, DAVA::Entity* _entity = NULL)
        : type(_type)
        , associatedData(_associatedData)
        , isFavorite(_isFavorite)
        , entity(_entity)
    {
    }

    QString realPath;
    QtPropertyData* associatedData;
    DAVA::Entity* entity;
    PropertyType type;
    bool isFavorite;
};

class PropertyEditor : public QtPropertyEditor
{
    Q_OBJECT

public:
    enum eViewMode : DAVA::uint32
    {
        VIEW_NORMAL,
        VIEW_ADVANCED,
        VIEW_FAVORITES_ONLY
    };

    PropertyEditor(QWidget* parent = 0, bool connectToSceneSignals = true);
    ~PropertyEditor();

    virtual void SetEntities(const SelectableGroup* selected);

    void SetViewMode(eViewMode mode);
    eViewMode GetViewMode() const;

    void SetFavoritesEditMode(bool set);
    bool GetFavoritesEditMode() const;

    bool IsFavorite(QtPropertyData* data) const;
    void SetFavorite(QtPropertyData* data, bool favorite);

    void LoadScheme(const DAVA::FilePath& path);
    void SaveScheme(const DAVA::FilePath& path);

public slots:
    void sceneActivated(SceneEditor2* scene);
    void sceneDeactivated(SceneEditor2* scene);
    void sceneSelectionChanged(SceneEditor2* scene, const SelectableGroup* selected, const SelectableGroup* deselected);
    void CommandExecuted(SceneEditor2* scene, const Command2* command, bool redo);

    void ActionEditComponent();
    void ActionEditMaterial();
    void ActionEditSoundComponent();
    void OnAddActionComponent();
    void OnAddStaticOcclusionComponent();
    void OnAddSoundComponent();
    void OnAddWaveComponent();
    void OnAddModelTypeComponent();
    void OnAddSkeletonComponent();
    void OnAddPathComponent();
    void OnAddRotationControllerComponent();
    void OnAddSnapToLandscapeControllerComponent();
    void OnAddWASDControllerComponent();
    void OnAddVisibilityComponent();
    void OnRemoveComponent();
    void OnTriggerWaveComponent();

    void ConvertToShadow();

    void DeleteRenderBatch();

    void RebuildTangentSpace();

    void CloneRenderBatchesToFixSwitchLODs();

    void ResetProperties();

private:
    QtPropertyData* CreateInsp(const DAVA::FastName& name, void* object, const DAVA::InspInfo* info);
    QtPropertyData* CreateInspMember(const DAVA::FastName& name, void* object, const DAVA::InspMember* member);
    QtPropertyData* CreateInspCollection(const DAVA::FastName& name, void* object, const DAVA::InspColl* collection);
    QtPropertyData* CreateClone(QtPropertyData* original);

    void ClearCurrentNodes();
    void ApplyModeFilter(QtPropertyData* parent);
    void ApplyFavorite(QtPropertyData* data);
    void ApplyCustomExtensions(QtPropertyData* data);

    void OnAddComponent(DAVA::Component::eType type);
    void OnAddComponent(DAVA::Component* component);

    void AddFavoriteChilds(QtPropertyData* parent);
    void RemFavoriteChilds(QtPropertyData* parent);

    bool IsInspViewAllowed(const DAVA::InspInfo* info) const;

    void OnItemEdited(const QModelIndex& index) override;
    void drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void mouseReleaseEvent(QMouseEvent* event) override;

    bool IsParentFavorite(const QtPropertyData* data) const;
    PropEditorUserData* GetUserData(QtPropertyData* data) const;

    QtPropertyToolButton* CreateButton(QtPropertyData* data, const QIcon& icon, const QString& tooltip);

    QString GetDefaultFilePath();

    void AddEntityProperties(DAVA::Entity* node, std::unique_ptr<QtPropertyData>& root,
                             std::unique_ptr<QtPropertyData>& curEntityData, bool isFirstInList);

private:
    LazyUpdater* propertiesUpdater = nullptr;
    QtPosSaver posSaver;
    QSet<QString> scheme;
    QtPropertyData* favoriteGroup;
    SelectableGroup curNodes;
    PropertyEditorStateHelper treeStateHelper;
    DAVA::Vector<std::unique_ptr<QtPropertyData>> favoriteList;
    eViewMode viewMode;
    bool favoritesEditMode;
};

#endif // __QT_PROPERTY_WIDGET_H__
