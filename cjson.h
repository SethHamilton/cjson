/*

cjson - a nice object driven JSON parser and serializer. 

Features:

  - while JSON will never be as easy as JSON is in JavaScript
    cjson is one of the easiers to use I've come across.
  - uses HeapStack (https://github.com/SethHamilton/HeapStack) to
    perform block allocations. All memory including cjson node objects
	are stored within the HeapStack. This eliminates the memory fragmentation
	that is typical found in DOM implemenations. HeapStack also eleminites
	all the overhead of allocating small objects and buffers (which is huge).
  - has xpath type functionality to find nodes by document path.
  - has many helpers to get, set, append, etc. when manually working
    on nodes.
  - Fast! It can Parse and Stringify at nearly the same speed (which is fast).
    On my Core i7 I was able to parse a heavily nest 185MB JSON file in 
	3930ms. I was able to serialize it out to a non-pretified 124MB JSON
	file in 4140ms. 
  - Easily incorporated into code that must call REST endpoints or where
    you want configuration in JSON rather than CONF formats.

The MIT License (MIT)

Copyright (c) 2015 Seth A. Hamilton

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#ifndef CJSON_H	
#define CJSON_H

#include <cstdio>
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>
#include "../heapstack/heapstack.h"

enum class cjsonType : int64_t { VOIDED, NUL, OBJECT, ARRAY, INT, DBL, STR, BOOL };

class cjson 
{
public:

	HeapStack* mem;

	cjsonType nodeType;
	char*   nodeName;

	// dataUnion uses the often ignored but always awesome
	// union feature of C++
	//
	// the union allows us to view an abitrary pointer of type dataUnion
	// as a any of the types within in. We can reference the values within
	// the union without having to do any other fussy type casting
	union dataUnion
	{
		char asStr;
		uint64_t asInt;
		double asDouble;
		bool asBool;
	};

	dataUnion*  nodeData;

	// if a doc or array it will have members
	//std::vector< cjson* > nodeMembers;
	
	// members are linked list of other nodes at this
	// document level.
	// this is how array and object values are stored
	cjson* membersHead;
	cjson* membersTail;
	int        memberCount;

	// next and previous sibling in to this members node list
	cjson* siblingPrev;
	cjson* siblingNext;

	cjson(HeapStack* MemObj);

	~cjson();

	/*
	-------------------------------------------------------------------------
	node information
	-------------------------------------------------------------------------
	*/
	
	cjsonType getType(); // returns the node type cjsonNode:: is the enum
	std::string getName(); // returns the node name or ""
	const char* getNameCstr(); // returns pointer to node name or NULL

	void setName(const char* newName);
	void setName(std::string newName);
	void setType(cjsonType Type);

	// test for node by xpath
	bool isNode(std::string xPath);
	bool hasName();

	/*
	-------------------------------------------------------------------------
	node construction / destruction

	Use MakeDocument() to create a root document node with a memory manager.

	Methods that create new nodes will call one of the following, these 
	can be called directly.

	Nodes are created detached from the document, but are associated with 
	the doucments memory manager. If you destroy the document unattached 
	node will also become invalid.

	Note: removeNode will mark a nodes type as voided. Nodes are not
	      actually pruned fromt the document. 
	      name and data will be NULLed.
	      nodes fo cjsonType::VOIDED will not be emitted by Stringify
		  VOIDED nodes will not be eimmited by functions that return
		  std::vectors.
		  Take care to avoid VOIDED nodes when iterating.
		  
	-------------------------------------------------------------------------
	*/

	cjson* createNode();
	cjson* createNode(cjsonType Type, const char* Name);
	cjson* createNode(cjsonType Type, std::string Name);
	void   removeNode();

	/*
	-------------------------------------------------------------------------
	node navigation
	-------------------------------------------------------------------------
	*/

	// get child node (member) names
	std::vector< std::string> getKeys();

	// returns all the child nodes (members) for this node 
	std::vector< cjson* > getNodes();

	// get a value AT index from an array or document
	cjson* at(int index);

	// search this node for (immediate) child of name
	cjson* find(const char* Name);
	cjson* find(std::string Name);
	
	// helper, append a value to an array
	/*
	-------------------------------------------------------------------------
	push funtions.

	these will push nodes with a specified value into the current
	nodes members lists (this is usually an array in this case).

	These are used by the Parse function and can also be used directly.

	result is the newly created cjson object (node).
	-------------------------------------------------------------------------
	*/
	cjson* push(int64_t Value);
	cjson* push(double Value);
	cjson* push(const char* Value);
	cjson* push(std::string Value);
	cjson* push(bool Value);
	cjson* push(cjson* Node); // push a node - returns provided node
	cjson* pushArray(); // append members with an array 
	cjson* pushObject(); 	// append members with object/sub-docuemnt 

	/*
	-------------------------------------------------------------------------
	key/value funtions.

	upsert functionality, will adds or update an existing key value pair 
	in a doc type node 
	
	i.e. "key": "value" or "key": #value, etc.
	-------------------------------------------------------------------------
	*/
	cjson* set(const char* Key, int64_t Value);
	cjson* set(std::string Key, int64_t Value);
	cjson* set(const char* Key, double Value);
	cjson* set(std::string Key, double Value);
	cjson* set(const char* Key, const char* Value);
	cjson* set(std::string Key, std::string Value);
	cjson* set(const char* Key, bool Value);
	cjson* set(std::string Key, bool Value);
	cjson* set(const char* Key); // sets "key": null
	cjson* set(std::string Key); // sets "key": null

	// adds a new array or returns an existing array key value type\
	// i.e. "key": []
	cjson* setArray(const char* Key);
	cjson* setArray(std::string Key);

	// adds a new or returns an existing document key value type
	// i.e. "key": {}
	cjson* setObject(const char* Key);
	cjson* setObject(std::string Key);

	// get number of items in an array
	int size();

	/*
	-------------------------------------------------------------------------
	replace value funtions.

	Will overwrite the current value (and type) for an existing node
	-------------------------------------------------------------------------
	*/
	void replace(int64_t Val);
	void replace(double Val);
	void replace(const char* Val);
	void replace(std::string Val);
	void replace(bool Val);
	void replace(); // sets value to NUll;
	

	/* 
	-------------------------------------------------------------------------
	Path Based Search Functions

    Thes are for searching a document and returning the value at that node.

	documents are searched using a path:

	i.e. /node/node/arrayindex/node

	a Default value is provided, if the node is not found the Default 
	will be returned.
	
	This is really useful when using json or config data where
	a document may be missing values, but defaults are required.	

	Note: Path is node relative, for full paths use your root document
	for nodes deeper into the document use a relative path.
	-------------------------------------------------------------------------
	*/

	int64_t xPath(std::string Path, int64_t Default);
	bool xPath(std::string Path, bool Default);
	double xPath(std::string Path, double Default);
	const char* xPath(const char* Path, const char* Default);
	std::string xPath(std::string Path, std::string Default);
	// returns a node or NULL;
	cjson* xPath(std::string Path);
		
	/*
	-------------------------------------------------------------------------
	Value Access Funtions

	These retreive the value at a node.

	Unlike JavaScript we have to be very aware of type.

	These functions returna true/false for success/failure and
	return the value (if successful) as a reference. 

	This is safe and allows for use in conditionals!
	
	i.e.

	int64_t someInt;
	double someDouble;

	if (someNode->isInt( someInt )) 
	   // do something
	else if (someNode->isDouble( someDouble ))
	   // do something else

	-------------------------------------------------------------------------
	*/
	
	bool isStringCstr(char* &Value);
	bool isString(std::string &Value);
	bool isInt(int64_t &Value);
	bool isDouble(double &Value);
	bool isBool(bool &Value);
	bool isNull();

	/*
	-------------------------------------------------------------------------
	Document import/export functions
	-------------------------------------------------------------------------
	*/

	static cjson* Parse(const char* JSON);
	static cjson* Parse(std::string JSON);
		
	// returns char* you must call delete[] on the result
	static char* cjson::StringifyCstr(cjson* N);
	// returns std::string (calls char* verison internally)
	// this version is slightly slower than the char* version
	// on multi-megabyte json documents
	static std::string cjson::Stringify(cjson* N);

	// completely free a document and all it's children
	// all nodes in the document become invalid immediately
	static void DisposeDocument(cjson* Document);
	// create a root node (with heapstack object).
	static cjson* MakeDocument();


private:
	// Each node can have children. These are implemented as 
	// linked list of nodes. The link properties as well as head
	// and tail properties are present in each node.
	// Link will set maintain membersHead and membersTail within
	// the node that calls link as well as maintain siblingNext
	// and siblingPrev for newNode and it's siblings.
	void Link(cjson* newNode);
	static cjson* ParseBranch(cjson* N, char* &cursor);

	// funtion used by xPath functions
	cjson* GetNodeByPath(std::string Path);
	// worker used stringifyC
	void cjson::Stringify_worker(cjson* N, char* &writer);


};


#endif CJSON_H