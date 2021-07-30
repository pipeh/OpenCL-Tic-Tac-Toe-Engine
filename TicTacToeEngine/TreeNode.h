#pragma once
#ifndef TreeNode_H
#define TreeNode_H

#include <string>
#include <vector>

class TreeNode{
private:
    int value;

    int* board;

    TreeNode* parent;

    std::vector<TreeNode*> children;

    bool isMax;

    bool isLeaf;

    bool isBranch;

    int countNodesRec(TreeNode* root, int& count);

public:
    TreeNode();
    TreeNode(int* board, bool indicator, bool max);

    void appendChild(TreeNode* child);
    void setParent(TreeNode* parent);

    void setIsLeaf(bool l);
    bool getIsLeaf();

    void setIsBranch(bool b);
    bool getIsBranch();

    void popBackChild();
    void removeChild(TreeNode* child);

    bool hasChildren();
    bool hasParent();

    TreeNode* getParent();
    TreeNode* getChild(int pos);
    std::vector<TreeNode*> getChilds();

    std::vector<TreeNode*> getBranches(int n, int d);
    std::vector<TreeNode*> getLeaves(int n, int d);

    int childrenNumber();
    int grandChildrenNum();

    int* getBoard();

    int getValue();
    void setValue(int v);

    bool getIsMax();

    void getNodesAtDepth(int d, int c, std::vector<TreeNode*> &selectedNodes);
};

#endif