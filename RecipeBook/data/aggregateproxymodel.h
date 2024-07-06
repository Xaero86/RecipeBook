#ifndef AGGREGATE_PROXY_MODEL_H
#define AGGREGATE_PROXY_MODEL_H

#include <QAbstractProxyModel>
#include <QMap>

class AggregateProxyModel : public QAbstractProxyModel
{
    Q_OBJECT

public:
    explicit AggregateProxyModel(QObject *parent = nullptr);

    virtual QStringList indexPath(const QModelIndex &index) const;

    virtual void setSourceModel(QAbstractItemModel *srcModel) override;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &proxyIndex, int role) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
    virtual QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;

protected:
    void invalidatePath();

protected slots:
    void sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles = QList<int>());
    void sourceRowsInserted(const QModelIndex &parent, int first, int last);
    void sourceRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last);
    void sourceRowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int destinationRow);
    void sourceRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int destinationRow);
    void sourceModelReset();

private:
    class TreeItem
    {
    public:
        TreeItem() : _parent(nullptr), _nodeData() {}
        TreeItem(TreeItem* parent, const QPersistentModelIndex &nodeData) : _parent(parent), _nodeData(nodeData) {}
        TreeItem(TreeItem* parent, const QString &nodeName) : _parent(parent), _nodeData(nodeName) {}
        virtual ~TreeItem() {_parent = nullptr; _nodeData = QVariant();}

        QModelIndex createPath(const QStringList &nodePath, AggregateProxyModel *model, const QModelIndex &myIndex);
        QVariant getPath(const QStringList &nodePath, const AggregateProxyModel *model, const QModelIndex &myIndex) const;
        void cleanPath(const QStringList &nodePath, AggregateProxyModel *model, const QModelIndex &myIndex);

        void addChild(const QModelIndex &nodeData);
        void addChild(TreeItem *child);
        int childIndex(const QModelIndex &nodeData);
        TreeItem *removeChild(const QModelIndex &nodeData);

        TreeItem* parent() const {return _parent;}
        bool isLeaf() const {return _nodeData.canConvert<QPersistentModelIndex>();}
        QVariant nodeData() const {return _nodeData;}
        int childCount() const {return int(_childs.size());}
        TreeItem* child(int index) const;
        int row() const;
        void clear();

    private:
        TreeItem*                              _parent;
        QVariant                               _nodeData;
        std::vector<TreeItem*>                 _childs;

        inline std::vector<TreeItem*>::const_iterator childIt(const QModelIndex &nodeData) const {
            return std::find_if(_childs.cbegin(), _childs.cend(),
                                [nodeData](const TreeItem *treeItem) {
                                    return treeItem->_nodeData.toPersistentModelIndex() == nodeData;
                                });
        }
        inline std::vector<TreeItem*>::const_iterator childIt(const QString &nodeName) const {
            return std::find_if(_childs.cbegin(), _childs.cend(),
                                [nodeName](const TreeItem *treeItem) {
                                    return treeItem->_nodeData.toString() == nodeName;
                                });
        }
    };
    std::unique_ptr<TreeItem>                  _root;
    QMap<QPersistentModelIndex, QStringList>   _knownPath;

    TreeItem* treeFromIndex(const QModelIndex &index) const;
    void refreshPath(const QModelIndex &index, bool force);
};

#endif // AGGREGATE_PROXY_MODEL_H
