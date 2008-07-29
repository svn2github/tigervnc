#ifndef __VIRTUALREG_H__
#define __VIRTUALREG_H__

// Key of registr with one parameter
struct ValueKey {
	char *hive;
	char *valueName;
};

class VirtualReg {
protected:
public:
	VirtualReg();
	~VirtualReg();
};

#endif // __VIRTUALREG_H__