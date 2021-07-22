#pragma once
#ifndef TreeNode_H
#define TreeNode_H

#include <string>
#include <vector>

class TreeNode{
private:
    int* board;

    TreeNode* parent;

    std::vector<TreeNode*> children;

    bool isLeaf;

    int countNodesRec(TreeNode* root, int& count);

public:
    TreeNode();
    TreeNode(int *board);

    void appendChild(TreeNode* child);
    void setParent(TreeNode* parent);

    void setIsLeaf(bool leaf);
    bool getIsLeaf();

    void popBackChild();
    void removeChild(int pos);

    bool hasChildren();
    bool hasParent();

    TreeNode* getParent();
    TreeNode* getChild(int pos);
    std::vector<TreeNode*> getChilds();

    TreeNode* getBranches(int n);
    TreeNode* getLeaves(int n);

    int childrenNumber();
    int grandChildrenNum();

    int* getBoard();
};

#endif