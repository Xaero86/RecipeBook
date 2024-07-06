#include "aggregateproxymodel.h"

#include <QFont>
#include <QTimer>
#include <algorithm>

AggregateProxyModel::AggregateProxyModel(QObject *parent)
    : QAbstractProxyModel{parent}, _root{new TreeItem}
{}

QStringList AggregateProxyModel::indexPath(const QModelIndex &) const
{
    return QStringList();
}

void AggregateProxyModel::invalidatePath()
{
    foreach (auto index, _knownPath.keys()) {
        refreshPath(index, true);
    }
}

void AggregateProxyModel::setSourceModel(QAbstractItemModel *srcModel)
{
    if (srcModel == sourceModel()) {
        return;
    }
    beginResetModel();
    if (sourceModel()) {
        disconnect(sourceModel(), nullptr, this, nullptr);
    }
    QAbstractProxyModel::setSourceModel(srcModel);
    connect(srcModel, &QAbstractItemModel::dataChanged, this, &AggregateProxyModel::sourceDataChanged);
    connect(srcModel, &QAbstractItemModel::rowsInserted, this, &AggregateProxyModel::sourceRowsInserted);
    connect(srcModel, &QAbstractItemModel::rowsAboutToBeRemoved, this, &AggregateProxyModel::sourceRowsAboutToBeRemoved);
    connect(srcModel, &QAbstractItemModel::rowsAboutToBeMoved, this, &AggregateProxyModel::sourceRowsAboutToBeMoved);
    connect(srcModel, &QAbstractItemModel::rowsMoved, this, &AggregateProxyModel::sourceRowsMoved);
    connect(srcModel, &QAbstractItemModel::modelReset, this, &AggregateProxyModel::sourceModelReset);
    endResetModel();
}

AggregateProxyModel::TreeItem* AggregateProxyModel::treeFromIndex(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return _root.get();
    } else {
        return static_cast<TreeItem*>(index.internalPointer());
    }
}

void AggregateProxyModel::refreshPath(const QModelIndex &index, bool force)
{
    if (_knownPath.contains(index)) {
        QStringList previousPath = _knownPath.value(index);
        QStringList currentPath = indexPath(index);
        if ((previousPath != currentPath) || force) {
            QVariant fromProxyParentVariant = _root->getPath(previousPath, this, QModelIndex());
            if (fromProxyParentVariant.canConvert<QModelIndex>()) {
                QModelIndex fromProxyParent = fromProxyParentVariant.toModelIndex();
                QModelIndex toProxyParent = _root->createPath(currentPath, this, QModelIndex());
                TreeItem* fromParentPtr = treeFromIndex(fromProxyParent);
                TreeItem* toParentPtr = treeFromIndex(toProxyParent);

                int childIndex = fromParentPtr->childIndex(index);
                if (childIndex != -1) {
                    beginMoveRows(fromProxyParent, childIndex, childIndex, toProxyParent, toParentPtr->childCount());
                    TreeItem* toMove = fromParentPtr->removeChild(index);
                    toParentPtr->addChild(toMove);
                    _knownPath[index] = currentPath;
                    endMoveRows();

                    _root->cleanPath(previousPath, this, QModelIndex());
                }
            }
        }
    }
}

QModelIndex AggregateProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }
    TreeItem* parentPtr = treeFromIndex(parent);
    if (!parentPtr->isLeaf() && (row < parentPtr->childCount())) {
        return createIndex(row, column, parentPtr->child(row));
    } else {
        return QModelIndex();
    }
}

QModelIndex AggregateProxyModel::parent(const QModelIndex &child) const
{
    TreeItem* childPtr = treeFromIndex(child);
    const TreeItem* parentPtr = childPtr->parent();
    if ((parentPtr == nullptr) || (parentPtr == _root.get())) {
        return QModelIndex();
    } else {
        return createIndex(parentPtr->row(), 0, parentPtr);
    }
}

int AggregateProxyModel::rowCount(const QModelIndex &parent) const
{
    TreeItem* parentPtr = treeFromIndex(parent);
    if (!parentPtr->isLeaf()) {
        return parentPtr->childCount();
    } else {
        return 0;
    }
}

int AggregateProxyModel::columnCount(const QModelIndex &parent) const
{
    QModelIndex parentSource = mapToSource(parent);
    if (parentSource.isValid()) {
        return sourceModel()->columnCount(parentSource);
    } else {
        return 1;
    }
}

QVariant AggregateProxyModel::data(const QModelIndex &proxyIndex, int role) const
{
    QModelIndex sourceIndex = mapToSource(proxyIndex);

    if (sourceIndex.isValid()) {
        return sourceModel()->data(sourceIndex, role);
    }

    if (!proxyIndex.isValid()) {
        return QVariant();
    }
    const TreeItem* itemPtr = static_cast<const TreeItem*>(proxyIndex.constInternalPointer());
    switch(role){
    case Qt::DisplayRole:
        return itemPtr->nodeData();
    case Qt::FontRole:
        QFont font;
        font.setBold(true);
        return font;
    }

    return QVariant();
}

Qt::ItemFlags AggregateProxyModel::flags(const QModelIndex &index) const
{
    QModelIndex sourceIndex = mapToSource(index);

    if (sourceIndex.isValid()) {
        return sourceModel()->flags(sourceIndex);
    }

    if (!index.isValid()) {
        return Qt::NoItemFlags;
    } else {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
}

QModelIndex AggregateProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (!proxyIndex.isValid()) {
        return QModelIndex();
    }
    const TreeItem* itemPtr = static_cast<const TreeItem*>(proxyIndex.constInternalPointer());
    if (itemPtr->isLeaf()) {
        return itemPtr->nodeData().toPersistentModelIndex();
    } else {
        return QModelIndex();
    }
}

QModelIndex AggregateProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceIndex.isValid()) {
        return QModelIndex();
    }
    QStringList path = indexPath(sourceIndex);
    QVariant proxySourceVariant = _root->getPath(path, this, QModelIndex());
    if (proxySourceVariant.canConvert<QModelIndex>()) {
        QModelIndex proxySource = proxySourceVariant.toModelIndex();
        TreeItem* sourcePtr = treeFromIndex(proxySource);
        int childIndex = sourcePtr->childIndex(sourceIndex);
        if (childIndex != -1) {
            return index(childIndex, 0, proxySource);
        }
    }
    return QModelIndex();
}

void AggregateProxyModel::sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles)
{
    if (!topLeft.isValid() || !bottomRight.isValid() || (topLeft.parent() != bottomRight.parent())) {
        return;
    }
    int minRow = std::min(topLeft.row(), bottomRight.row());
    int maxRow = std::max(topLeft.row(), bottomRight.row());
    int minColumn = std::min(topLeft.column(), bottomRight.column());
    int maxColumn = std::max(topLeft.column(), bottomRight.column());
    for (int row = minRow; row <= maxRow; row++) {
        for (int column = minColumn; column <= maxColumn; column++) {
            QModelIndex sourceIndex = sourceModel()->index(row, column, topLeft.parent());
            QModelIndex proxyIndex = mapFromSource(sourceIndex);
            emit dataChanged(proxyIndex, proxyIndex, roles);
            refreshPath(sourceIndex, false);
        }
    }
}

void AggregateProxyModel::sourceRowsInserted(const QModelIndex &parent, int first, int last)
{
    if (parent.isValid()) {
        return;
    }
    for (int i = first; i <= last; i++) {
        QPersistentModelIndex newData = QPersistentModelIndex(sourceModel()->index(i, 0, parent));
        if (!newData.isValid()) {
            continue;
        }
        QStringList path = indexPath(newData);
        QModelIndex proxyParent = _root->createPath(path, this, QModelIndex());
        TreeItem* parentPtr = treeFromIndex(proxyParent);
        beginInsertRows(proxyParent, parentPtr->childCount(), parentPtr->childCount());
        parentPtr->addChild(newData);
        _knownPath[newData] = path;
        endInsertRows();
    }
}

void AggregateProxyModel::sourceRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
    for (int i = first; i <= last; i++) {
        QModelIndex dataToRemove = sourceModel()->index(i, 0, parent);
        if (!dataToRemove.isValid()) {
            continue;
        }
        QStringList path = indexPath(dataToRemove);
        QVariant proxyParentVariant = _root->getPath(path, this, QModelIndex());
        if (proxyParentVariant.canConvert<QModelIndex>()) {
            QModelIndex proxyParent = proxyParentVariant.toModelIndex();
            TreeItem* parentPtr = treeFromIndex(proxyParent);
            int index = parentPtr->childIndex(dataToRemove);
            if (index != -1) {
                beginRemoveRows(proxyParent, index, index);
                TreeItem* toRemove = parentPtr->removeChild(dataToRemove);
                _knownPath.remove(dataToRemove);
                endRemoveRows();
                _root->cleanPath(path, this, QModelIndex());
                delete toRemove;
            }
        }
    }
}

void AggregateProxyModel::sourceRowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int)
{
    if (sourceParent.isValid()) {
        return;
    }
    if (destinationParent.isValid()) {
        sourceRowsAboutToBeRemoved(sourceParent, sourceStart, sourceEnd);
    } else {
        // TODO
    }
}

void AggregateProxyModel::sourceRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int destinationRow)
{
    if (destinationParent.isValid()) {
        return;
    }
    if (!sourceParent.isValid()) {
        sourceRowsInserted(destinationParent, destinationRow, destinationRow + (sourceEnd - sourceStart));
    } else {
        // TODO
    }
}

void AggregateProxyModel::sourceModelReset()
{
    beginResetModel();
    _root->clear();
    _knownPath.clear();
    endResetModel();
}

QModelIndex AggregateProxyModel::TreeItem::createPath(const QStringList &nodePath, AggregateProxyModel *model, const QModelIndex &myIndex)
{
    if (nodePath.isEmpty()) {
        return myIndex;
    } else {
        QString currentNode = nodePath.front();
        auto it = childIt(currentNode);
        if (it == _childs.cend()) {
            it = std::find_if(_childs.cbegin(), _childs.cend(),
                              [currentNode](const TreeItem *treeItem) {
                                  return treeItem->isLeaf() || treeItem->_nodeData.toString().compare(currentNode, Qt::CaseInsensitive) > 0;
                              });
            int index = std::distance(_childs.cbegin(), it);
            model->beginInsertRows(myIndex, index, index);
            it = _childs.insert(it, new TreeItem(this, currentNode));
            model->endInsertRows();
        }
        QModelIndex childIndex = model->index(std::distance(_childs.cbegin(), it), 0, myIndex);
        return (*it)->createPath(nodePath.mid(1), model, childIndex);
    }
}

QVariant AggregateProxyModel::TreeItem::getPath(const QStringList &nodePath, const AggregateProxyModel *model, const QModelIndex &myIndex) const
{
    if (nodePath.isEmpty()) {
        return myIndex;
    } else {
        auto it = childIt(nodePath.front());
        if (it != _childs.cend()) {
            QModelIndex childIndex = model->index(std::distance(_childs.cbegin(), it), 0, myIndex);
            return (*it)->getPath(nodePath.mid(1), model, childIndex);
        } else {
            return false;
        }
    }
}

void AggregateProxyModel::TreeItem::cleanPath(const QStringList &nodePath, AggregateProxyModel *model, const QModelIndex &myIndex)
{
    if (nodePath.isEmpty()) {
        return;
    } else {
        auto it = childIt(nodePath.front());
        if (it != _childs.end()) {
            QModelIndex childIndex = model->index(std::distance(_childs.cbegin(), it), 0, myIndex);
            (*it)->cleanPath(nodePath.mid(1), model, childIndex);
            if ((*it)->childCount() == 0) {
                int index = std::distance(_childs.cbegin(), it);
                model->beginRemoveRows(myIndex, index, index);
                TreeItem* toRemove = *it;
                _childs.erase(it);
                model->endRemoveRows();
                delete toRemove;
            }
        }
    }
}

void AggregateProxyModel::TreeItem::addChild(const QModelIndex &nodeData)
{
    _childs.push_back(new TreeItem(this, nodeData));
}

void AggregateProxyModel::TreeItem::addChild(TreeItem *child)
{
    child->_parent = this;
    _childs.push_back(child);
}

int AggregateProxyModel::TreeItem::childIndex(const QModelIndex &nodeData)
{
    auto it = childIt(nodeData);
    if (it != _childs.cend()) {
        return std::distance(_childs.cbegin(), it);
    } else {
        return -1;
    }
}

AggregateProxyModel::TreeItem* AggregateProxyModel::TreeItem::removeChild(const QModelIndex &nodeData)
{
    auto it = childIt(nodeData);
    if (it != _childs.cend()) {
        TreeItem* toRemove = *it;
        _childs.erase(it);
        return toRemove;
    } else {
        return nullptr;
    }
}

AggregateProxyModel::TreeItem* AggregateProxyModel::TreeItem::child(int index) const
{
    if (index < childCount()) {
        return _childs.at(index);
    }
    return nullptr;
}

int AggregateProxyModel::TreeItem::row() const
{
    if (_parent == nullptr) {
        return 0;
    }
    const auto it = std::find(_parent->_childs.cbegin(), _parent->_childs.cend(),this);

    if (it != _parent->_childs.cend()) {
        return std::distance(_parent->_childs.cbegin(), it);
    }
    return -1;
}

void AggregateProxyModel::TreeItem::clear()
{
    foreach (auto child, _childs) {
        delete child;
    }
    _childs.clear();
}
