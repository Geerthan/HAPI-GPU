/********************************************
 *
 *              Node.h
 *
 *  Generic scene graph node.
 *
 ********************************************/
#ifndef _NODEH
#define _NODEH

#include <vector>

#define NONE				0
#define GEOMETRYNODE		1
#define TRANSFORMATIONNODE	10
#define COLOURNODE			20
#define STATICNODE			21

class Node {
public:
	Node(int type);
	~Node();
	int getType();
	void addChild(Node* child);
	void removeChild(Node* child);
	Node* getChild(int i);
	int count();
protected:
	int _type;
	std::vector<Node*> _childList;
};
#endif