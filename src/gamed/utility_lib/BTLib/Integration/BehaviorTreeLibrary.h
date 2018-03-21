#ifndef GAMED_UTILITY_LIB_BTLIB_BEHAVIORTREELIBRARY_H_
#define GAMED_UTILITY_LIB_BTLIB_BEHAVIORTREELIBRARY_H_


namespace BTLib {

class BTLeafFactory;
/**
 * BehaviorTreeLibrary class.
 * The objects of this class can be used for the following purposes (in this order):
 *  1. Initialize the behavior tree library.
 *  2. Register custom leaf nodes.
 *  3. Finalize the behavior tree library.
 * Usage: 
 *  1. Create a subclass of this, and override the method registerLeafNodes(), including the
 *      macro calls that create the leaf node ccreator functions. (see 
 *      BTLeafFactory::registerLeafNodes() for and example on how to use this macros).
 *  2. When starting the application, call to the function Init();
 *  3. When closing the application, call to Finalize() so the library can free the memory it is usung.
 */ 
class BehaviorTreeLibrary
{
public:
	// Constructor of this Object
	BehaviorTreeLibrary() { }

	// Destructor of this Object
	~BehaviorTreeLibrary() { }   

	/// **** THREAD UNSAFE **** ///
	// Initializes the behavior tree library.
	bool Init(BTLeafFactory* factory);
	// Closes the behavior tree library.
	void Finalize();


protected:
	//! This virtual function is called by Init() when initializating the Lirary.
	virtual void registerLeafNodes(BTLeafFactory* factory) = 0;


private:
	BTLeafFactory* leafFactory_;
};

} // namespace BTLib

#endif // GAMED_UTILITY_LIB_BTLIB_BEHAVIORTREELIBRARY_H_
