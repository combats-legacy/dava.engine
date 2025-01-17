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


#ifndef __QTREEVIEW_STATE_HELPER__H__
#define __QTREEVIEW_STATE_HELPER__H__

#include "DAVAEngine.h"
#include <QTreeView>

// The purpose of QTreeViewStateHelper is to store the current Collapsed/Expanded
// state for the tree view and expand it back.
namespace DAVA
{
template <class T>
class QTreeViewStateHelper
{
public:
    QTreeViewStateHelper(QTreeView* treeView)
    {
        this->treeView = treeView;
    }

    virtual ~QTreeViewStateHelper() = default;

    virtual void SaveTreeViewState(bool needCleanupStorage = true)
    {
        if (!treeView || !treeView->model())
        {
            return;
        }

        SaveTreeViewState(expandedItems, needCleanupStorage);
    }

    virtual void RestoreTreeViewState()
    {
        if (!treeView || !treeView->model())
        {
            return;
        }

        RestoreTreeViewStateRecursive(treeView, QModelIndex(), expandedItems);
    }

    // Extended versions - save/restore Tree View state to external storage.
    void SaveTreeViewState(Set<T>& storage, bool needCleanupStorage)
    {
        if (!treeView || !treeView->model())
        {
            return;
        }

        if (needCleanupStorage)
        {
            storage.clear();
        }

        SaveTreeViewStateRecursive(treeView, QModelIndex(), storage);
    }

    void RestoreTreeViewState(Set<T>& storage)
    {
        if (!treeView || !treeView->model())
        {
            return;
        }

        if (storage.empty())
        {
            return;
        }

        RestoreTreeViewStateRecursive(treeView, QModelIndex(), storage);
    }

    bool IsTreeStateStorageEmpty()
    {
        return this->expandedItems.empty();
    }

    void CleanupTreeStateStorage()
    {
        this->expandedItems.cleanup();
    }

protected:
    // This method must be overriden to return persistent data to particular model index.
    virtual T GetPersistentDataForModelIndex(const QModelIndex& modelIndex) = 0;

    void SaveTreeViewStateRecursive(const QTreeView* treeView, const QModelIndex& parent,
                                    Set<T>& storage)
    {
        QAbstractItemModel* model = treeView->model();
        int rowCount = model->rowCount(parent);

        for (int i = 0; i < rowCount; ++i)
        {
            QPersistentModelIndex idx = model->index(i, 0, parent);
            if (!idx.isValid())
            {
                continue;
            }

            // Store only expanded items to reduce the memory footprint.
            T persistentData = GetPersistentDataForModelIndex(idx);
            if (persistentData != NULL)
            {
                if (treeView->isExpanded(idx))
                {
                    expandedItems.insert(persistentData);
                }
                else
                {
                    expandedItems.erase(persistentData);
                }
            }

            SaveTreeViewStateRecursive(treeView, idx, storage);
        }
    }

    void RestoreTreeViewStateRecursive(QTreeView* treeView, const QModelIndex& parent,
                                       Set<T>& storage)
    {
        QAbstractItemModel* model = treeView->model();
        int rowCount = model->rowCount(parent);

        for (int i = 0; i < rowCount; ++i)
        {
            QModelIndex idx = model->index(i, 0, parent);
            if (!idx.isValid())
            {
                continue;
            }

            T persistentData = GetPersistentDataForModelIndex(idx);
            if (persistentData == NULL)
            {
                continue;
            }

            // Only expanded items are stored, so if item isn't in the list - it is collapsed.
            typename Set<T>::iterator iter = storage.find(persistentData);
            treeView->setExpanded(idx, (iter != storage.end()));

            RestoreTreeViewStateRecursive(treeView, idx, storage);
        }
    }

private:
    // QTreeView to attach to.
    QTreeView* treeView;

    // Expanded items.
    Set<T> expandedItems;
};
};

#endif //__QTREEVIEW_STATE_HELPER__H__
