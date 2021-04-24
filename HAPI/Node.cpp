/**********************************
 *
 *        Node.cpp
 *
 *  Implementation of the Node class
 *
 **********************************/

#include "Node.h"

Node::Node(int type) {

	_type = type;

}

Node::~Node() {

}

int Node::getType() {

	return(_type);

}

int Node::count() {

	return(_childList.size());

}

void Node::addChild(Node* child) {

	_childList.push_back(child);

}

Node* Node::getChild(int i) {

	return(_childList[i]);

}

void Node::removeChild(Node* child) {
	
}