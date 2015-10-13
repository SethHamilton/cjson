/* 
-----------------------------------------------------------------
 cjson - Copyright 2015, Seth A. Hamilton

 refer to cjson.h for comments.
 -----------------------------------------------------------------
*/

#include "cjson.h"
#include <sstream>
#include <iomanip>
	
// Split - an std::string split function. 
// some of these should just be part of the stl by now.
// I'm sure the folks that write the stl would make a much
// nicer version than this one, but this one works!
void Split(std::string Source, char Token, std::vector< std::string > &result )
{
	result.clear(); // clear it just incase it's being reused

	if (!Source.length())
		return;


	size_t length = Source.length();
	size_t start = 0;
	size_t end = Source.find(Token, 0);

	size_t diff;

	while (end != -1)
	{
		// removes empty splits 
		if (end - start == 0)
		{
			while (start != length && Source[start] == Token)
				start++;

			if ((end = Source.find(Token, start)) == -1) break;
		}

		diff = end - start;

		if (diff > 0)
			result.push_back(Source.substr(start, diff));

		start += diff + 1;
		end = Source.find(Token, start);
	}

	// anything stray at the end, if so append it.
	if (length - start > 0)
		result.push_back(Source.substr(start, length - start));

	if (result.size() == 0)
		result.push_back(Source);

}

// SkipJunk - helper function for document parser. Skips spaces, line breaks, etc.
// updates cursor as a reference.
void SkipJunk(char* &cursor)
{
	while (*cursor == ' ' ||
		*cursor == '\r' ||
		*cursor == '\n' ||
		*cursor == '\t')
		++cursor;
}

// ParseNumeric - helper function to advance cursor past number
// and return number as std::string
__inline std::string ParseNumeric( char* &cursor, bool &isDouble )
{

	char* cursorStart = cursor;

	isDouble = false;

	// is a number, decimal and possibliy negagive
	while (*cursor >= '-' && *cursor <= '9')
	{
		if (*cursor == '.')
			isDouble = true;
		cursor++;
	}

	return std::string(cursorStart, cursor - cursorStart);

}

cjson* cjson::ParseBranch( cjson* N, char* &cursor )
{

	if (!*cursor)
		return cjson::MakeDocument();

	if (N == NULL)
	{
		SkipJunk( cursor );

		if (*cursor != '[' && *cursor != '{')
		{
			return cjson::MakeDocument();
		}

		if (*cursor == '{')
		{
			N = cjson::MakeDocument();
		}
		else
		{
			N = cjson::MakeDocument();
			N->setType(cjsonType::ARRAY);
		}

		cursor++;
	}

	std::string Name = "";

	char* start;
	size_t len;

	while (*cursor)
	{
		
		// this is a number out at the main loop, so we are appending an array probably

		if (*cursor == '}' || *cursor == ']')
		{
			cursor++;
			return N;
		}

		// this is a docum without a string identifier, or an document in an array likely
		if (*cursor == '{')
		{
			cursor++;
			cjson* A = N->pushObject();
			cjson::ParseBranch( A, cursor );
			//cursor++; // move past closing condition;
		}
		// this is an array without a string identifier, or an array in an array likely
		else if (*cursor == '[')
		{
			cursor++;
			cjson* A = N->pushArray();
			cjson::ParseBranch( A, cursor );
			//cursor++; // move past closing condition;
		}
		else if (*cursor == '-' || (*cursor >= '0' && *cursor <= '9')) // number or neg number)
		{
			
			bool isDouble = false;
			std::string Value = ParseNumeric( cursor, isDouble );

			if (isDouble)
			{
				char* endp;
				double D = strtod( Value.c_str(), &endp ); 
				N->push( D );
			}
			else
			{
				char* endp;
				int64_t LL = strtoll( Value.c_str(), &endp, 10 ); 
				N->push( LL );
			}

		}
		else if (*cursor == '"')
		{
			cursor++;

			start = cursor;
			while (*cursor != 0)
			{
				if (*cursor == '"')
					break;
				if (*cursor == '\\') 
					cursor++;			
				cursor++;
			}

			len = cursor - start;
			Name.assign( start, len );

			cursor++;
					
			SkipJunk( cursor );

			// this is a comman right after a string, so we are appending an array of strings
			if (*cursor == ',' || *cursor == ']') {
				
				N->push( Name );

			}
			else
			if (*cursor == ':') {
				
				cursor++;
				SkipJunk( cursor );

				// we have a nested document
				if (*cursor == '{')
				{
					cursor++;

					cjson* D = N->setObject( Name );
					cjson::ParseBranch( D, cursor );
					//cursor++; // move past closing condition;
					continue;

				}
				else
				// we have a nested array
				if (*cursor == '[')
				{
					cursor++;

					cjson* A = N->setArray( Name );
					cjson::ParseBranch( A, cursor );
					//cursor++; // move past closing condition;
					continue;
				}
				else
				if (*cursor == '"')
				{
					cursor++;

					start = cursor;

					while (*cursor != 0)
					{
						if (*cursor == '"')
							break;

						if (*cursor == '\\') 
							cursor++;			

						cursor++;
					}

					len = cursor - start;

					std::string Value = "";
					Value.assign( start, len );

					//cursor++;

					N->set( Name, Value );					
				}
				else
				if (*cursor == '-' || (*cursor >= '0' && *cursor <= '9')) // number or neg number
				{

					bool isDouble = false;
					std::string Value = ParseNumeric( cursor, isDouble );

					if (isDouble)
					{
						char* endp;
						double D = strtod( Value.c_str(), &endp ); 
						N->set( Name, D );					
					}
					else
					{
						char* endp;
						int64_t LL = strtoll( Value.c_str(), &endp, 10 ); 
						N->set( Name, LL );					
					}

					continue;
				}
				else
				if (*cursor == 'N' || *cursor == 'n') // skip null
				{
					N->set(Name);
					cursor += 4;
					continue;
				}
				else
				if (*cursor == 'u' || *cursor == 'U') // skip undefined
				{
					cursor += 8;
				}
				else
				if (*cursor == 't' || *cursor == 'f')
				{

					bool TF = false;

					if (*cursor == 't')
					{
						TF = true;
						cursor += 3;
					}
					else
						cursor += 4;

					N->set( Name, TF );					
				}
				
				cursor++;

			}


		}
		else
		cursor++;


	};

	return N;
	
}


/*
  Member functions for cjson
*/

cjson::cjson(HeapStack* MemObj) :
	nodeType(cjsonType::VOIDED),
	nodeName(NULL),
	nodeData(NULL),
	siblingPrev(NULL),
	siblingNext(NULL),
	membersHead(NULL),
	membersTail(NULL),
	memberCount(0),
	mem(MemObj)
{
};


cjson::~cjson()
{
	// this destructor does nothing and is never
	// called because we are using "placement new" for
	// our allocations
};

cjsonType cjson::getType()
{
	return nodeType;
};

void cjson::setName(const char* newName)
{
	if (newName)
	{
		size_t len = strlen(newName) + 1;
		this->nodeName = mem->newPtr(len);
		memcpy(this->nodeName, newName, len);
	}
};

void cjson::setName(std::string newName)
{
	setName(newName.c_str());
};

void cjson::setType(cjsonType Type)
{
	nodeType = Type;
}


cjson* cjson::createNode()
{
	char* nodePtr = mem->newPtr(sizeof(cjson));
	return new (nodePtr) cjson(mem);
};

cjson* cjson::createNode(cjsonType Type, const char* Name)
{
	cjson* newNode = createNode();
	newNode->setName(Name);
	newNode->nodeType = Type;
	return newNode;
};

cjson* cjson::createNode(cjsonType Type, std::string Name)
{
	cjson* newNode = createNode();
	newNode->setName(Name.c_str());
	newNode->nodeType = Type;
	return newNode;
};

void cjson::removeNode()
{
	nodeType = cjsonType::VOIDED;
	nodeName = NULL;
	nodeData = NULL;
	membersHead = NULL;
	membersTail = NULL;
	memberCount = 0;		 
};

bool cjson::isNode(std::string xPath)
{
	return (GetNodeByPath(xPath)) ? true : false;
};


bool cjson::hasName()
{
	return (nodeName && *((char*)nodeName) != 0) ? true : false;
};

std::vector< std::string> cjson::getKeys()
{
	// allocate a list of the required size for all the node names
	int count = this->size();
	std::vector< std::string > names;
	names.reserve(count);

	auto n = membersHead;

	while (n)
	{
		if (n->nodeName && n->nodeType != cjsonType::VOIDED)
			names.push_back(std::string(n->nodeName));
		n = n->siblingNext;
	}

	return names;
}

std::vector< cjson* > cjson::getNodes()
{

	int count = this->size();
	std::vector< cjson* > array;
	array.reserve(count);

	// only return for ARRAY and OBJECT type nodes
	if (nodeType != cjsonType::ARRAY &&
		nodeType != cjsonType::OBJECT)
		return array; // return an empty array

	cjson* n = membersHead;

	while (n)
	{
		if (n->nodeType != cjsonType::VOIDED)
			array.push_back(n);
		n = n->siblingNext;
	}

	return array;
}

cjson* cjson::at(int index)
{

	// linked lists don't really do random access
	// so this won't be the fasted function ever written
	//
	// if repeated iteration is needed using one of the 
	// array type functions would proably be best
	//
	// i.e. GetNodes();

	int iter = 0;
	cjson* n = membersHead;

	while (n)
	{

		if (iter == index)
			return n;

		n = n->siblingNext;
		iter++;
	}

	return NULL;
};

cjson* cjson::find(const char* Name)
{

	int Count = this->size();

	cjson* n = membersHead;

	while (n)
	{
		if (n->nodeName && strcmp(n->nodeName, Name) == 0)
			return n;

		n = n->siblingNext;
	}

	return NULL;
}

cjson* cjson::find(std::string Name)
{
	return find(Name.c_str());
}

// helper, append a value to an array
cjson* cjson::push(int64_t Value)
{
	cjson* newNode = createNode();
	// initialize the node
	newNode->replace(Value);
	Link(newNode);
	return newNode;
}

// helper, append a value to an array
cjson* cjson::push(double Value)
{
	cjson* newNode = createNode();
	// initialize the node
	newNode->replace(Value);
	Link(newNode);
	return newNode;
};

// helper, append a value to an array
cjson* cjson::push(const char* Value)
{
	cjson* newNode = createNode();
	// initialize the node
	newNode->replace(Value);
	Link(newNode);
	return newNode;
};

// helper, append a value to an array
cjson* cjson::push(std::string Value)
{
	cjson* newNode = createNode();
	// initialize the node
	newNode->replace(Value);
	Link(newNode);
	return newNode;
};

// helper, append a value to an array
cjson* cjson::push(bool Value)
{
	cjson* newNode = createNode();
	// initialize the node
	newNode->replace(Value);
	Link(newNode);
	return newNode;
};

// helper, append a value to an array
cjson* cjson::push(cjson* Node)
{
	Link(Node);
	return Node;
};


// append a nested array onto the array
cjson* cjson::pushArray()
{
	cjson* newNode = createNode();
	// initialize the node
	newNode->nodeType = cjsonType::ARRAY;

	Link(newNode);
	return newNode;
};

// append a nested document onto the array
cjson* cjson::pushObject()
{
	cjson* newNode = createNode();

	// initialize the node
	newNode->nodeType = cjsonType::OBJECT;

	Link(newNode);
	return newNode;
};


// adds or updates an existing key value pair in a doc type node
cjson* cjson::set(const char* Key, int64_t Value)
{
	cjson* Node = find(Key);

	if (!Node)
	{
		cjson* newNode = createNode();

		newNode->setName(Key);
		Link(newNode);

		Node = newNode;
	}

	Node->replace(Value);
	return Node;
};

cjson* cjson::set(std::string Key, int64_t Value)
{
	return set(Key.c_str(), Value);
}

// adds or updates an existing key value pair in a doc type node
cjson* cjson::set(const char* Key, double Value)
{
	cjson* Node = find(Key);

	if (!Node)
	{
		cjson* newNode = createNode();

		newNode->setName(Key);

		Link(newNode);

		Node = newNode;
	}

	Node->replace(Value);
	return Node;
};

cjson* cjson::set(std::string Key, double Value)
{
	return set(Key.c_str(), Value);
}

// adds or updates an existing key value pair in a doc type node
cjson* cjson::set(const char* Key, const char* Value)
{
	cjson* Node = find(Key);

	if (!Node)
	{
		cjson* newNode = createNode();

		newNode->setName(Key);
		Link(newNode);

		Node = newNode;
	}

	Node->replace(Value);
	return Node;
};


cjson* cjson::set(std::string Key, std::string Value)
{
	return set(Key.c_str(), Value.c_str());
};

cjson* cjson::set(const char* Key, bool Value)
{
	cjson* Node = find(Key);

	if (!Node)
	{
		cjson* newNode = createNode();

		newNode->setName(Key);
		Link(newNode);

		Node = newNode;
	}

	Node->replace(Value);
	return Node;
};

cjson* cjson::set(std::string Key, bool Value)
{
	return set(Key.c_str(), Value);
};


// adds null
cjson* cjson::set(const char* Key)
{
	cjson* Node = find(Key);

	if (!Node)
	{
		cjson* newNode = createNode();

		newNode->setName(Key);
		Link(newNode);
		Node = newNode;
	}

	Node->nodeType = cjsonType::NUL;
	return Node;
}

cjson* cjson::set(std::string Key)
{
	return set(Key.c_str());
}


// adds or updates an existing key value pair in a doc type node

// adds a new array or returns an existing array key value type
cjson* cjson::setArray(const char* Key)
{
	cjson* Node = find(Key);

	if (!Node)
	{
		cjson* newNode = createNode();

		newNode->setName(Key);
		newNode->nodeType = cjsonType::ARRAY;
		Link(newNode);

		return newNode;
	}

	return Node;
};

cjson* cjson::setArray(std::string Key)
{
	return setArray(Key.c_str());
};

// adds a new or returns an existing document key value type
cjson* cjson::setObject(const char* Key)
{
	cjson* Node = find(Key);

	if (!Node)
	{
		cjson* newNode = createNode();

		newNode->setName(Key);
		newNode->nodeType = cjsonType::OBJECT;
		Link(newNode);

		return newNode;
	}

	return Node;
}

cjson* cjson::setObject(std::string Key)
{
	return setObject(Key.c_str());
}

// get number of items in an array
int cjson::size()
{
	return memberCount;
}

void cjson::replace(int64_t Val)
{
	nodeType = cjsonType::INT;
	nodeData = (dataUnion*)mem->newPtr(sizeof(Val));
	nodeData->asInt = Val;
}

void cjson::replace(double Val)
{
	nodeType = cjsonType::DBL;
	nodeData = (dataUnion*)mem->newPtr(sizeof(Val));
	nodeData->asDouble = Val;
}

void cjson::replace(const char* Val)
{
	nodeType = cjsonType::STR;

	size_t len = strlen(Val) + 1;
	char* textPtr = mem->newPtr(len);
	// we are going to copy the string to textPtr
	// but we have to point nodeData to this as well
	nodeData = (dataUnion*)textPtr;
	memcpy(textPtr, Val, len);

}

void cjson::replace(std::string Val)
{
	replace(Val.c_str());
}

void cjson::replace()
{
	nodeType = cjsonType::NUL;
	nodeData = NULL;	
}


void cjson::replace(bool Val)
{
	nodeType = cjsonType::BOOL;
	nodeData = (dataUnion*)mem->newPtr(sizeof(Val));
	nodeData->asBool = Val;
}

int64_t cjson::xPath(std::string Path, int64_t Default)
{
	cjson* N = GetNodeByPath(Path);
	if (N && N->nodeType == cjsonType::INT)
		return N->nodeData->asBool;
	return Default;
}

bool cjson::xPath(std::string Path, bool Default)
{
	cjson* N = GetNodeByPath(Path);
	if (N && N->nodeType == cjsonType::BOOL)
		return N->nodeData->asBool;
	return Default;
}

double cjson::xPath(std::string Path, double Default)
{
	cjson* N = GetNodeByPath(Path);
	if (N && N->nodeType == cjsonType::DBL)
		return N->nodeData->asDouble;
	return Default;
}

const char* cjson::xPath(const char* Path, const char* Default)
{
	cjson* N = GetNodeByPath(Path);
	if (N && N->nodeType == cjsonType::STR)
		return &N->nodeData->asStr;
	return Default;
}

std::string cjson::xPath(std::string Path, std::string Default)
{
	const char* res = xPath((char*)Path.c_str(), Default.c_str());
	return std::string(res);
}

cjson* cjson::xPath(std::string Path)
{
	cjson* N = GetNodeByPath(Path);
	if (N) return N;
	return NULL;
};


// helpers for serialization
std::string cjson::getName()
{
	if (nodeName)
		return std::string((char*)nodeName);
	else
		return std::string("");
};

const char* cjson::getNameCstr()
{
	if (nodeName)
		return (const char*)nodeName;
	else
		return NULL;
};


bool cjson::isStringCstr( char* &Value )
{
	if (nodeType == cjsonType::STR)
	{
		Value = (char*)nodeData;
		return true;
	}
	return false;
};

bool cjson::isString( std::string &Value )
{
	if (nodeType == cjsonType::STR)
	{
		Value = (char*)nodeData;
		return true;
	}
	return false;
};

bool cjson::isInt(int64_t &Value)
{
	if (nodeType == cjsonType::INT)
	{
		Value = nodeData->asInt;
		return true;
	}
	return false;
};

bool cjson::isDouble(double &Value)
{
	if (nodeType == cjsonType::DBL)
	{
		Value = nodeData->asDouble;
		return true;
	}
	return false;
};

bool cjson::isBool(bool &Value)
{
	if (nodeType == cjsonType::BOOL)
	{
		Value = nodeData->asBool;
		return true;
	}
	return false;
};

bool cjson::isNull()
{
	if (nodeType == cjsonType::NUL)
		return true;
	return false;
};



cjson* cjson::Parse( const char* JSON )
{
	char* cursor = (char*)JSON;
	return cjson::ParseBranch( NULL, cursor );
}

cjson* cjson::Parse(std::string JSON)
{
	return cjson::Parse(JSON.c_str());
};

char* cjson::StringifyCstr(cjson* N)
{
	char* buffer = new char[N->mem->getBytes()];
	char* writer = buffer;

	N->Stringify_worker(N, writer);

	*writer = 0;
	return buffer;
}

std::string cjson::Stringify(cjson* N)
{
	return std::string(StringifyCstr(N));
}


void cjson::DisposeDocument( cjson* Document )
{
	delete Document->mem;
};

cjson* cjson::MakeDocument()
{
	HeapStack* mem = new HeapStack();
	void* Data = mem->newPtr(sizeof(cjson));

	// we are going to allocate this node using "palcement new"
	// in the HeapStack
	cjson* newNode = new (Data) cjson(mem);

	newNode->setName("__root__");
	newNode->setType(cjsonType::OBJECT);

	return newNode;
};

cjson* cjson::GetNodeByPath(std::string Path)
{

	cjson* N = this;

	std::vector<std::string> parts;
	Split(Path, '/', parts);

	for (int i = 0; i < parts.size(); i++)
	{

		switch (N->getType())
		{
		case cjsonType::OBJECT:
		{
			cjson* Next = N->find(parts[i]);

			if (!Next)
				return NULL;

			N = Next;

		}
		break;
		case cjsonType::ARRAY:
		{
			int Index = atoi(parts[i].c_str());

			if (Index < 0 || Index >= N->size())
				return NULL;

			N = N->at(Index);

			if (!N)
				return NULL;

		}
		break;
		}

	}

	return N;

}

// internal add member
void cjson::Link(cjson* newNode)
{

	// members are basically children directly owned
	// by the current node. They are stored as a linked
	// list maintained by the parent node.
	// nodes can also see their siblings via siblingPrev and
	// siblingNext

	if (!membersHead)
	{
		membersHead = newNode;
		membersTail = newNode;
		newNode->siblingNext = NULL;
		newNode->siblingPrev = NULL;
	}
	else
	{
		cjson* lastTail = membersTail;
		// set the current tails sibling to the node		
		membersTail->siblingNext = newNode;
		// new nodes previous is the last tail
		newNode->siblingPrev = lastTail;
		// set the current tail to the node;
		membersTail = newNode;
	}

	memberCount++;

};

// helper for Stringify_worder
__forceinline void emitText(char* &writer, const char* text)
{
	while (*text)
	{
		*writer = *text;
		++writer;
		++text;
	}
}

// helper for Stringify_worder
__forceinline void emitText(char* &writer, const char text)
{
	*writer = text;
	++writer;
}


void cjson::Stringify_worker(cjson* N, char* &writer)
{

	std::string pad = "";
	std::string padshort = "";

	bool AppendDoc = false;

	switch (N->nodeType)
	{
	case cjsonType::NUL:
		if (N->hasName())
		{
			emitText(writer, '"');
			emitText(writer, N->getNameCstr());
			emitText(writer, "\":");
		}

		emitText(writer, "null");

		break;
	case cjsonType::INT:
		if (N->hasName())
		{
			emitText(writer, '"');
			emitText(writer, N->getNameCstr());
			emitText(writer, "\":");
		}

		writer += sprintf(writer, "%ld", N->nodeData->asInt);
		//emitText(writer, std::to_string(N->nodeData->asInt).c_str());

		break;
	case cjsonType::DBL:
		if (N->hasName())
		{
			emitText(writer, '"');
			emitText(writer, N->getNameCstr());
			emitText(writer, "\":");
		}

		if (N->nodeData->asDouble == 0)
		{
			emitText(writer, "0.0");
		}
		else
		{
			//doc.setf(std::ios_base::fixed, std::ios::floatfield);
			//doc << std::setprecision(9) << N->nodeData->asDouble;
			//emitText(writer, std::to_string(N->nodeData->asDouble).c_str());
			writer += sprintf(writer, "%0.7f", N->nodeData->asDouble);
		}

		break;
	case cjsonType::STR:
		if (N->hasName())
		{
			emitText(writer, '"');
			emitText(writer, N->getNameCstr());
			emitText(writer, "\":");
		}

		emitText(writer, '"');
		emitText(writer, (char*)N->nodeData);
		emitText(writer, '"');

		break;
	case cjsonType::BOOL:
		if (N->hasName())
		{
			emitText(writer, '"');
			emitText(writer, N->getNameCstr());
			emitText(writer, "\":");
		}

		emitText(writer, (N->nodeData->asBool) ? "true" : "false");

		break;
	case cjsonType::ARRAY:
	{
		int Count = N->size();

		if (!N->hasName() || N->getName() == "__root__")
			emitText(writer, '[');
		else
		{
			emitText(writer, '"');
			emitText(writer, N->getNameCstr());
			emitText(writer, "\":[");
		}

		auto members = N->getNodes();
		for (int i = 0; i < Count; i++)
		{
			if (i) emitText(writer, ',');

			cjson::Stringify_worker(members[i], writer);
		}

		emitText(writer, ']');

	}
	break;

	case cjsonType::OBJECT:
	{
		int Count = N->size();

		std::string name = N->getName();

		if (!N->hasName() || N->getName() == "__root__")
			emitText(writer, '{');
		else
		{
			emitText(writer, '"');
			emitText(writer, N->getNameCstr());
			emitText(writer, "\":{");
		}


		auto members = N->getNodes();

		for (int i = 0; i < Count; i++)
		{
			if (i) emitText(writer, ',');;

			cjson::Stringify_worker(members[i], writer);
		}

		emitText(writer, '}');

	}

	break;

	}

}

