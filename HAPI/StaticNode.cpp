/******************************************
 *
 *       StaticNode.cpp
 *
 *  Implementation of the static node.
 *
 *****************************************/
#include "InterferencePattern.h"
#include "StaticNode.h"

StaticNode::StaticNode() : Node(STATICNODE) {

	_dirty = 1;

}

StaticNode::~StaticNode() {

}

void StaticNode::setInterferencePattern(InterferencePattern& pattern) {

	_pattern = pattern;
	_dirty = 0;

}

InterferencePattern StaticNode::getInterferencePattern() {

	return(_pattern);

}