#pragma once
/***************************************************************
 *                          Device.h
 *
 *  A singleton object that describes the device that the
 *  hologram is being generated from.  By default this
 *  information is loaded from the device.halo file in
 *  the current directory.  This file is in JSON format.
 *
 **********************************************************/


#include "jansson.h"

class Device {
private:
	static Device *instance;
	static char *filename;
	Device();
	void loadFile(char *filename);
	int pixelsW;
	int pixelsH;
	double sizeW;
	double sizeH;
	double _minDepth;
	double _maxDepth;
	double _left;
	double _right;
	double _top;
	double _bottom;
	bool _diamond;
public:
	static Device *getInstance();
	static void setFileName(char *filename);
	int pixelsWidth() {
		return(pixelsW);
	};
	int pixelsHeight() {
		return(pixelsH);
	};
	double sizeWide() {
		return(sizeW);
	};
	double sizeHeight() {
		return(sizeH);
	};
	double left() {
		return(_left);
	}
	double right() {
		return(_right);
	}
	double top() {
		return(_top);
	}
	double bottom() {
		return(_bottom);
	}
	double deviceWidth() {
		return(pixelsW * sizeW);
	};
	double deviceHeight() {
		return(pixelsH * sizeH);
	};
	double minDepth() { return(_minDepth); };
	double maxDepth() { return(_maxDepth); };
	double diamond() { return(_diamond); };
};