#ifndef _DERIVED_H
#define _DERIVED_H

#include <memory>
#include <cassert>

///a wrapper class to make it easy to put derived classes into STL containers
template<typename T_BASECLASS>
class DERIVED
{
	private:
		T_BASECLASS * ptr;
	
	public:
		DERIVED() : ptr(NULL) {}
		DERIVED(T_BASECLASS * newobj) {ptr = newobj;}
		DERIVED(const DERIVED & other) : ptr(NULL) {assert(!other.ptr);}
		DERIVED & operator= (T_BASECLASS * newobj) {if (ptr) delete ptr;ptr=newobj;return *this;}
		~DERIVED() {if (ptr) delete ptr;}
		T_BASECLASS * Get() {return ptr;}
		const T_BASECLASS * Get() const {return ptr;}
		const T_BASECLASS * GetReadOnly() const {return ptr;}
		T_BASECLASS * operator-> () {assert(ptr);return ptr;}
		const T_BASECLASS * operator-> () const {assert(ptr);return ptr;}
		T_BASECLASS & operator* ()  { assert(ptr);return *ptr; }
		const T_BASECLASS & operator* () const { assert(ptr);return *ptr; }
		DERIVED & operator= (const DERIVED & other) {assert(!other.ptr); return *this;}
};

#endif
