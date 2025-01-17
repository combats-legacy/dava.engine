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


#ifndef __QT_SCENE_TREE_H__
#define __QT_SCENE_TREE_H__

#include <QWidget>
#include <QTreeView>
#include <QTableView>
#include <QTimer>
#include <QPointer>

#include "Scene/SceneSignals.h"
#include "DockSceneTree/SceneTreeModel.h"
#include "DockSceneTree/SceneTreeDelegate.h"

class LazyUpdater;
class SceneTree : public QTreeView
{
    Q_OBJECT

public:
    explicit SceneTree(QWidget* parent = 0);

protected:
    void dropEvent(QDropEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;

private slots:
    void ShowContextMenu(const QPoint& pos);
    void SetFilter(const QString& filter);

    void CollapseSwitch();
    void CollapseAll();

    void SceneActivated(SceneEditor2* scene);
    void SceneDeactivated(SceneEditor2* scene);
    void SceneSelectionChanged(SceneEditor2* scene, const SelectableGroup* selected, const SelectableGroup* deselected);
    void SceneStructureChanged(SceneEditor2* scene, DAVA::Entity* parent);
    void CommandExecuted(SceneEditor2* scene, const Command2* command, bool redo);

    void ParticleLayerValueChanged(SceneEditor2* scene, DAVA::ParticleLayer* layer);

    void TreeSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void TreeItemClicked(const QModelIndex& index);
    void TreeItemDoubleClicked(const QModelIndex& index);
    void TreeItemCollapsed(const QModelIndex& index);
    void TreeItemExpanded(const QModelIndex& index);

    void SyncSelectionToTree();
    void SyncSelectionFromTree();

private:
    void GetDropParams(const QPoint& pos, QModelIndex& index, int& row, int& col);

    void EmitParticleSignals();

    void ExpandFilteredItems();
    void BuildExpandItemsSet(QSet<QModelIndex>& indexSet, const QModelIndex& parent = QModelIndex());

    void UpdateTree();
    void PropagateSolidFlag();
    void PropagateSolidFlagRecursive(QStandardItem* root);

    class BaseContextMenu;
    class EntityContextMenu;
    class ParticleLayerContextMenu;
    class ParticleForceContextMenu;
    class ParticleEmitterContextMenu;
    class ParticleInnerEmitterContextMenu;

private:
    QPointer<SceneTreeModel> treeModel;
    QPointer<SceneTreeFilteringModel> filteringProxyModel;
    SceneTreeDelegate* treeDelegate = nullptr;
    LazyUpdater* treeUpdater = nullptr;
    bool isInSync = false;
};

#endif // __QT_SCENE_TREE_H__
