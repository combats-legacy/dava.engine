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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"

#include "Classes/Qt/Main/ModificationWidget.h"
#include "Classes/Qt/Tools/QtWaitDialog/QtWaitDialog.h"
#include "Classes/Qt/Scene/SceneEditor2.h"
#include "Classes/Qt/Main/RecentMenuItems.h"
#include "Classes/Qt/NGTPropertyEditor/PropertyPanel.h"
#include "Classes/Beast/BeastProxy.h"

#include "DAVAEngine.h"

#include "core_generic_plugin/interfaces/i_component_context.hpp"

#include <QMainWindow>
#include <QDockWidget>
#include <QPointer>

class AddSwitchEntityDialog;
class Request;
class QtLabelWithActions;
class HangingObjectsHeight;
class DeveloperTools;
class VersionInfoWidget;

class DeviceListController;
class SpritesPackerModule;
class QtMainWindow
: public QMainWindow
  ,
  public DAVA::Singleton<QtMainWindow>
{
    Q_OBJECT

    static const int GLOBAL_INVALIDATE_TIMER_DELTA = 1000;

signals:
    void GlobalInvalidateTimeout();

    void TexturesReloaded();

public:
    explicit QtMainWindow(IComponentContext& ngtContext, QWidget* parent = 0);
    ~QtMainWindow();

    Ui::MainWindow* GetUI();
    SceneTabWidget* GetSceneWidget();
    SceneEditor2* GetCurrentScene();

    bool OpenScene(const QString& path);
    bool SaveScene(SceneEditor2* scene);
    bool SaveSceneAs(SceneEditor2* scene);

    void SetGPUFormat(DAVA::eGPUFamily gpu);
    DAVA::eGPUFamily GetGPUFormat();

    void WaitStart(const QString& title, const QString& message, int min = 0, int max = 100);
    void WaitSetMessage(const QString& messsage);
    void WaitSetValue(int value);
    void WaitStop();

    void BeastWaitSetMessage(const QString& messsage);
    bool BeastWaitCanceled();

    void EnableGlobalTimeout(bool enable);

    // qt actions slots
public slots:
    void OnProjectOpen();
    void OnProjectClose();
    void OnSceneNew();
    void OnSceneOpen();
    void OnSceneSave();
    void OnSceneSaveAs();
    void OnSceneSaveToFolder();
    void OnSceneSaveToFolderCompressed();
    void OnRecentFilesTriggered(QAction* recentAction);
    void OnRecentProjectsTriggered(QAction* recentAction);
    void ExportMenuTriggered(QAction* exportAsAction);
    void OnImportSpeedTreeXML();
    void RemoveSelection();

    void OnUndo();
    void OnRedo();

    void OnEditorGizmoToggle(bool show);
    void OnViewLightmapCanvas(bool show);
    void OnAllowOnSceneSelectionToggle(bool allow);
    void OnShowStaticOcclusionToggle(bool show);
    void OnEnableVisibilitySystemToggle(bool enabled);
    void OnRefreshVisibilitySystem();
    void OnFixVisibilityFrame();
    void OnReleaseVisibilityFrame();

    void OnEnableDisableShadows(bool enable);

    void OnReloadTextures();
    void OnReloadTexturesTriggered(QAction* reloadAction);

    void OnSelectMode();
    void OnMoveMode();
    void OnRotateMode();
    void OnScaleMode();
    void OnPivotCenterMode();
    void OnPivotCommonMode();
    void OnManualModifMode();
    void OnPlaceOnLandscape();
    void OnSnapToLandscape();
    void OnResetTransform();
    void OnLockTransform();
    void OnUnlockTransform();

    void OnCenterPivotPoint();
    void OnZeroPivotPoint();

    void OnMaterialEditor();
    void OnTextureBrowser();
    void OnSceneLightMode();

    void OnCubemapEditor();
    void OnImageSplitter();

    void OnAddLandscape();
    void OnAddVegetation();
    void OnLightDialog();
    void OnCameraDialog();
    void OnEmptyEntity();
    void OnAddWindEntity();
    void OnAddPathEntity();

    void OnUserNodeDialog();
    void OnSwitchEntityDialog();
    void OnParticleEffectDialog();
    void On2DCameraDialog();
    void On2DSpriteDialog();
    void OnAddEntityFromSceneTree();

    void OnShowSettings();
    void OnOpenHelp();

    void OnSaveHeightmapToImage();
    void OnSaveTiledTexture();
    void OnTiledTextureRetreived(DAVA::Landscape* landscape, DAVA::Texture* landscapeTexture);

    void OnConvertModifiedTextures();

    void OnCloseTabRequest(int tabIndex, Request* closeRequest);

    void OnBeastAndSave();

    void OnBuildStaticOcclusion();
    void OnInavalidateStaticOcclusion();

    void OnLandscapeEditorToggled(SceneEditor2* scene);
    void OnForceFirstLod(bool);
    void OnCustomColorsEditor();
    void OnHeightmapEditor();
    void OnRulerTool();
    void OnTilemaskEditor();
    void OnNotPassableTerrain();
    void OnWayEditor();

    void OnObjectsTypeChanged(QAction* action);
    void OnObjectsTypeChanged(int type);

    void OnHangingObjects();
    void OnHangingObjectsHeight(double value);

    void OnMaterialLightViewChanged(bool);
    void OnCustomQuality();

    void OnReloadShaders();

    void OnSwitchWithDifferentLODs(bool checked);

    void OnGenerateHeightDelta();

    void OnBatchProcessScene();

    void OnSnapCameraToLandscape(bool);

    void SetupTitle();

    void RestartParticleEffects();
    bool SetVisibilityToolEnabledIfPossible(bool);
    void SetLandscapeInstancingEnabled(bool);

protected:
    bool eventFilter(QObject* object, QEvent* event) override;
    bool ShouldClose(QCloseEvent* e);

    void SetupMainMenu();
    void SetupToolBars();
    void SetupStatusBar();
    void SetupDocks();
    void SetupActions();
    void SetupShortCuts();

    void StartGlobalInvalidateTimer();

    void RunBeast(const QString& outputPath, BeastProxy::eBeastMode mode);

    bool IsAnySceneChanged();

    void DiableUIForFutureUsing();
    void SynchronizeStateWithUI();

    bool SelectCustomColorsTexturePath();

    static void SetActionCheckedSilently(QAction* action, bool checked);

    void OpenProject(const DAVA::FilePath& projectPath);

    void OnSceneSaveAsInternal(bool saveWithCompressed);

    void SaveAllSceneEmitters(SceneEditor2* scene) const;

private slots:
    void ProjectOpened(const QString& path);
    void ProjectClosed();

    void SceneCommandExecuted(SceneEditor2* scene, const Command2* command, bool redo);
    void SceneActivated(SceneEditor2* scene);
    void SceneDeactivated(SceneEditor2* scene);
    void SceneSelectionChanged(SceneEditor2* scene, const SelectableGroup* selected, const SelectableGroup* deselected);

    void OnGlobalInvalidateTimeout();
    void EditorLightEnabled(bool enabled);
    void OnSnapToLandscapeChanged(SceneEditor2* scene, bool isSpanToLandscape);
    void UnmodalDialogFinished(int);

    void DebugVersionInfo();
    void DebugColorPicker();
    void DebugDeviceList();
    void OnConsoleItemClicked(const QString& data);

private:
    Ui::MainWindow* ui;
    QtWaitDialog* waitDialog;
    QtWaitDialog* beastWaitDialog;
    QPointer<QDockWidget> dockActionEvent;
    QPointer<QDockWidget> dockConsole;

    bool globalInvalidate;

    ModificationWidget* modificationWidget;

    QComboBox* objectTypesWidget;

    AddSwitchEntityDialog* addSwitchEntityDialog;
    HangingObjectsHeight* hangingObjectsWidget;

    void EnableSceneActions(bool enable);
    void EnableProjectActions(bool enable);
    void UpdateConflictingActionsState(bool enable);
    void UpdateModificationActionsState();
    void UpdateWayEditor(const Command2* command, bool redo);

    void LoadViewState(SceneEditor2* scene);
    void LoadUndoRedoState(SceneEditor2* scene);
    void LoadModificationState(SceneEditor2* scene);
    void LoadEditorLightState(SceneEditor2* scene);
    void LoadGPUFormat();
    void LoadLandscapeEditorState(SceneEditor2* scene);
    void LoadObjectTypes(SceneEditor2* scene);
    void LoadHangingObjects(SceneEditor2* scene);
    void LoadMaterialLightViewMode();

    bool SaveTilemask(bool forAllTabs = true);

    // Landscape editor specific
    // TODO: remove later -->
    bool LoadAppropriateTextureFormat();
    bool IsSavingAllowed();
    // <--

    //Need for any debug functionality
    QPointer<DeveloperTools> developerTools;
    QPointer<VersionInfoWidget> versionInfoWidget;

    QPointer<DeviceListController> deviceListController;

    RecentMenuItems recentFiles;
    RecentMenuItems recentProjects;

    IComponentContext& ngtContext;
    PropertyPanel propertyPanel;
    std::unique_ptr<SpritesPackerModule> spritesPacker;

private:
    struct EmitterDescriptor
    {
        EmitterDescriptor(DAVA::ParticleEmitter* _emitter, DAVA::ParticleLayer* layer, DAVA::FilePath path, DAVA::String name)
            : emitter(_emitter)
            , ownerLayer(layer)
            , yamlPath(path)
            , entityName(name)
        {
        }

        DAVA::ParticleEmitter* emitter = nullptr;
        DAVA::ParticleLayer* ownerLayer = nullptr;
        DAVA::FilePath yamlPath;
        DAVA::String entityName;
    };

    void CollectEmittersForSave(DAVA::ParticleEmitter* topLevelEmitter, DAVA::List<EmitterDescriptor>& emitters, const DAVA::String& entityName) const;
};


#endif // MAINWINDOW_H
