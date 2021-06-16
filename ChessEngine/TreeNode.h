#pragma once
#ifndef TreeNode_H
#define TreeNode_H

#include <string>
#include <vector>

class TreeNode{
private:
    int board [64];

    TreeNode* parent;

    std::vector<TreeNode*> children;

    int countNodesRec(TreeNode* root, int& count);

public:
    TreeNode();
    TreeNode(int *board);

    void appendChild(TreeNode* child);
    void setParent(TreeNode* parent);

    void popBackChild();
    void removeChild(int pos);

    bool hasChildren();
    bool hasParent();

    TreeNode* getParent();
    TreeNode* getChild(int pos);

    int childrenNumber();
    int grandChildrenNum();

    int* getBoard();
};

#endif