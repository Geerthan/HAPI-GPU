/**********************************
 *
 *        Static Node
 *
 *  Representation of the static
 *  scene graph node.
 *
 **********************************/
#ifndef _STATICNODE
#define _STATICNODE
#include "Node.h"

class StaticNode : public Node {
public:
	StaticNode();
	~StaticNode();
	void setInterferencePattern(InterferencePattern& pattern);
	InterferencePattern getInterferencePattern();
	int isDirty() { return(_dirty); };
protected:
	InterferencePattern _pattern;
	int _dirty;
};

#endif _STATICNODE