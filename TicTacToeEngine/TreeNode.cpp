#include "TreeNode.h"
#include <string>
#include <vector>
#include <iostream>

#define MAX 1000;
#define MIN -1000;

TreeNode::TreeNode() {
    value = NULL;
    board = NULL;
    parent = NULL;
    isLeaf = true;
    isMax = true;
}

TreeNode::TreeNode(int* b, bool i, bool m) {
    board = b;
    parent = NULL;
    isMax = m;

    if (i) {
        isLeaf = true;
        isBranch = false;
    }
    else {
        isLeaf = false;
        isBranch = true;
    }

    if (m) {
        value = MIN;
    }
    else {
        value = MAX;
    }
}

int TreeNode::countNodesRec(TreeNode* root, int& count) {
    TreeNode* parent = root;
    TreeNode* child = NULL;

    for (int it = 0; it < parent->childrenNumber(); it++)
    {
        child = parent->getChild(it);
        count++;
        //std::cout<<child->getTextContent()<<" Number : "<<count<<std::endl;
        if (child->childrenNumber() > 0)
        {
            countNodesRec(child, count);
        }
    }

    return count;
}

std::vector<TreeNode*> TreeNode::getBranches(int n, int d) {
    std::vector<TreeNode*> selectedNodes;
    this->getNodesAtDepth(d, 0, selectedNodes);
    std::vector<TreeNode*> branches;
    int counter = 0;

    for (int i = 0; i < selectedNodes.size(); i++) {
        TreeNode* node = selectedNodes[i];
        if (node->getIsBranch()) {
            branches.push_back(node);
            counter++;
        }
        if (counter == n) {
            break;
        }
    }
    return branches;
}

std::vector<TreeNode*> TreeNode::getLeaves(int n, int d) {
    std::vector<TreeNode*> selectedNodes;
    this->getNodesAtDepth(d, 0, selectedNodes);
    std::vector<TreeNode*> leaves;
    int counter = 0;

    for (int i = 0; i < selectedNodes.size(); i++) {
        TreeNode* node = selectedNodes[i];
        if (node->getIsLeaf()) {
            leaves.push_back(node);
            counter++;
        }
        if (counter == n) {
            break;
        }
    }
    return leaves;
}

void TreeNode::setIsLeaf(bool l) {
    isLeaf = l;
}

bool TreeNode::getIsLeaf() {
    return isLeaf;
}

void TreeNode::setIsBranch(bool b) {
    isBranch = b;
}

bool TreeNode::getIsBranch() {
    return isBranch;
}

void TreeNode::appendChild(TreeNode* child) {
    child->setParent(this);
    children.push_back(child);
}

void TreeNode::setParent(TreeNode* theParent) {
    parent = theParent;
}

void TreeNode::popBackChild() {
    children.pop_back();
}

void TreeNode::removeChild(TreeNode* child) {
    if (children.size() > 0) {
        int childIndex = std::find(children.begin(), children.end(), child) - children.begin();
        children.erase(children.begin() + childIndex);
    }
    else {
        children.pop_back();
    }
}

bool TreeNode::hasChildren() {
    if (children.size() > 0)
        return true;
    else
        return false;
}

bool TreeNode::hasParent() {
    if (parent != NULL)
        return true;
    else
        return false;
}

TreeNode* TreeNode::getParent() {
    return parent;
}

TreeNode* TreeNode::getChild(int pos) {
    if (children.size() < pos)
        return NULL;
    else
        return children[pos];
}

std::vector<TreeNode*> TreeNode::getChilds() {
    return children;
}

int TreeNode::childrenNumber() {
    return children.size();
}

int TreeNode::grandChildrenNum() {
    int t = 0;

    if (children.size() < 1)
    {
        return 0;
    }

    countNodesRec(this, t);

    return t;
}

int* TreeNode::getBoard() {
    return board;
}

int TreeNode::getValue() {
    return value;
}

void TreeNode::setValue(int v) {
    value = v;
}

bool TreeNode::getIsMax() {
    return isMax;
}

void TreeNode::getNodesAtDepth(int d, int c, std::vector<TreeNode*> &selectedNodes) {
    if (d == c) {
        selectedNodes.push_back(this);
    }
    else {
        std::vector<TreeNode*> childs = this->getChilds();
        for (int i = 0; i < childs.size(); i++) {
            childs[i]->getNodesAtDepth(d, c + 1, selectedNodes);
        }
    }
}