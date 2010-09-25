#ifndef QUADTREE_H
#define QUADTREE_H

#include <QDataStream>
#include <QSet>

template<typename T>
struct QuadtreeNode {
    QuadtreeNode *children[4];
    T value;
    bool leaf;

    enum {
        NorthWest = 0,
        NorthEast = 1,
        SouthWest = 2,
        SouthEast = 3
    };

    friend QDataStream &operator <<(QDataStream &stream, QuadtreeNode<T> &node)
    {
        stream << node.leaf;

        if (node.leaf) {
            stream << node.value;
        } else {
            for (int i = 0; i < 4; ++i)
                stream << *node.children[i];
        }

        return stream;
    }

    friend QDataStream &operator >>(QDataStream &stream, QuadtreeNode<T> &node)
    {
        stream >> node.leaf;

        if (node.leaf) {
            stream >> node.value;
        } else {
            for (int i = 0; i < 4; ++i) {
                node.children[i] = new QuadtreeNode<T>;
                stream >> *node.children[i];
            }
        }

        return stream;
    }
};

template<typename T>
class Quadtree {
public:
    typedef QuadtreeNode<T> Node;

    template<typename T>
    friend QDataStream &operator <<(QDataStream &stream, const Quadtree<T> &tree);

    template<typename T>
    friend QDataStream &operator >>(QDataStream &stream, Quadtree<T> &tree);

    Quadtree(uint sidelength, T initialValue)
        : mRoot(NULL), mSidelength(sidelength)
    {
        mRoot = new Node;
        mRoot->leaf = true;
        mRoot->value = initialValue;
    }

    Quadtree()
        : mRoot(NULL), mSidelength(1)
    {
        mRoot = new Node;
        mRoot->leaf = true;
    }

    ~Quadtree()
    {
        deleteNode(mRoot);
    }

    void set(int x, int y, const T &value)
    {
        Node *n = mRoot;

        int sidelength = mSidelength;
        int centerX = sidelength / 2 - 1;
        int centerY = centerX;

        while (!n->leaf) {
            int child = quadrant(x, y, centerX, centerY, sidelength);
            n = n->children[child];
        }

        while (sidelength > 1) {
            splitNode(n);

            // Choose the correct child
            int child = quadrant(x, y, centerX, centerY, sidelength);
            n = n->children[child];
        }

        n->value = value;
    }

    const T &get(int x, int y) const
    {
        Node *n = mRoot;

        int sidelength = mSidelength;
        int centerX = sidelength / 2 - 1;
        int centerY = centerX;

        while (!n->leaf) {
            int child = quadrant(x, y, centerX, centerY, sidelength);
            n = n->children[child];
        }

        return n->value;
    }

    void compact()
    {
        QList<Node*> canidates;
        canidates << mRoot;

        while (!canidates.isEmpty()) {
            Node *n = canidates.takeFirst();

            if (!n->leaf && !compactNode(n)) {
                for (int i = 0; i < 4; ++i)
                    canidates << n->children[i];
            }
        }
    }

private:

    T findFirstLeaf(Node *n)
    {
        while (!n->leaf) {
            n = n->children[0];
        }

        return n->value;
    }

    bool compactNode(Node *n)
    {
        if (n->leaf)
            return false; // Nothing changes for leafs

        T value = findFirstLeaf(n);

        // Cancels if not all sub-nodes of n have the same value
        if (!checkValue(n, value))
            return false;

        for (int i = 0; i < 4; ++i) {
            deleteNode(n->children[i]);
            n->children[i] = 0;
        }

        n->leaf = true;
        n->value = value;

        return true;
    }

    static int quadrant(int x, int y, int &centerX, int &centerY, int &sidelength)
    {
        int quadrant = 0;

        sidelength /= 2;

        if (x <= centerX) {
            centerX -= sidelength / 2;
        } else {
            centerX += sidelength / 2;
            quadrant++;
        }

        if (y <= centerY) {
            centerY -= sidelength / 2;
        } else {
            centerY += sidelength / 2;
            quadrant += 2;
        }

        return quadrant;
    }

    void splitNode(Node *node)
    {
        node->leaf = false;
        for (int i = 0; i < 4; ++i) {
            Node *child = new Node;
            child->leaf = true;
            child->value = node->value;
            node->children[i] = child;
        }
    }

    static void deleteNode(Node *startNode)
    {
        QList<Node*> deleteQueue;
        deleteQueue << startNode;

        while (!deleteQueue.isEmpty()) {
            Node *n = deleteQueue.takeFirst();

            if (!n->leaf) {
                for (int i = 0; i < 4; ++i)
                    deleteQueue << n->children[i];
            }

            delete n;
        }
    }

    bool checkValue(const Node *startNode, const T value) const
    {
        QList<const Node*> checkQueue;
        checkQueue << startNode;

        while (!checkQueue.isEmpty()) {
            const Node *n = checkQueue.takeFirst();

            if (!n->leaf) {
                for (int i = 0; i < 4; ++i)
                    checkQueue << n->children[i];
            } else if (n->value != value) {
                return false;
            }
        }

        return true;
    }

    Node *mRoot;
    int mSidelength;
};

template<typename T>
inline QDataStream &operator <<(QDataStream &stream, const Quadtree<T> &tree)
{
    stream << tree.mSidelength << *tree.mRoot;
    return stream;
}

template<typename T>
inline void insertValues(const QuadtreeNode<T> *node, QSet<T> &values) {
    if (node->leaf) {
        values.insert(node->value);
    } else {
        for (int i = 0; i < 4; ++i)
            insertValues(node->children[i], values);
    }
}


template<typename T>
inline void writeNodeWithDictionary(QDataStream &stream, const QuadtreeNode<T> *node, const QHash<T,uint> &dictionary)
{
    stream << node->leaf;

    if (node->leaf) {
        stream << dictionary[node->value];
    } else {
        for (int i = 0; i < 4; ++i)
            writeNodeWithDictionary(stream, node->children[i], dictionary);
    }
}

template<typename T>
inline void writeTreeWithDictionary(QDataStream &stream, int sidelength, const QuadtreeNode<T> *root) {
    QSet<T> values;
    insertValues(root, values);

    QList<T> dictionary = values.toList();
    QHash<T, uint> dictionaryMap;

    for (int i = 0; i < dictionary.size(); ++i) {
        dictionaryMap.insert(dictionary[i], i);
    }

    stream << sidelength << dictionary;

    writeNodeWithDictionary(stream, root, dictionaryMap);
}

template<>
inline QDataStream &operator <<(QDataStream &stream, const Quadtree<QString> &tree)
{
    writeTreeWithDictionary(stream, tree.mSidelength, tree.mRoot);
    return stream;
}

template<typename T>
inline QDataStream &operator >>(QDataStream &stream, Quadtree<T> &tree)
{
    if (!tree.mRoot->leaf) {
        for (int i = 0; i < 4; ++i)
            Quadtree<T>::deleteNode(tree.mRoot->children[i]);
    }
    stream >> tree.mSidelength >> *tree.mRoot;
    return stream;
}

template<typename T>
inline void readNodeWithDictionary(QDataStream &stream, QuadtreeNode<T> *node, const QList<T> &dictionary)
{
    stream >> node->leaf;

    if (node->leaf) {
        uint index;
        stream >> index;
        node->value = dictionary[index];
    } else {
        for (int i = 0; i < 4; ++i) {
            node->children[i] = new QuadtreeNode<T>();
            readNodeWithDictionary(stream, node->children[i], dictionary);
        }
    }
}

template<>
inline QDataStream &operator >>(QDataStream &stream, Quadtree<QString> &tree)
{
    if (!tree.mRoot->leaf) {
        for (int i = 0; i < 4; ++i)
            Quadtree<QString>::deleteNode(tree.mRoot->children[i]);
    }

    QList<QString> dictionary;

    stream >> tree.mSidelength >> dictionary;

    readNodeWithDictionary(stream, tree.mRoot, dictionary);

    return stream;
}

typedef Quadtree<bool> BoolQuadtree;
typedef Quadtree<QString> StringQuadtree;
typedef Quadtree<qint8> CharQuadtree;

#endif // QUADTREE_H
