##cjson
**A nice object driven JSON parser and serializer for C++.**

Features:

- while JSON will never be as easy as JSON is in JavaScript cjson is one of the easiers to use I've come across. 
- uses HeapStack (https://github.com/SethHamilton/HeapStack) to perform block allocations. All memory including cjson node objects are stored within the HeapStack. This eliminates the memory fragmentation	that is typical found in DOM implemenations. HeapStack also eleminites all the overhead of allocating small objects and buffers (which is huge).
- has xpath type functionality to find nodes by document path.
- has many helpers to get, set, append, etc. when manually working on nodes.
- Fast! It can Parse and Stringify at nearly the same speed (which is fast).On my Core i7 I was able to parse a heavily nest 185MB JSON file in 3930ms. I was able to serialize it out to a non-pretified 124MB JSON	file in 4140ms. 
- Easily incorporated into code that must call REST endpoints or where you want configuration in JSON rather than CONF formats.

Note:

Make sure to get my [HeapStack](https://github.com/SethHamilton/HeapStack) tool, as this will require it.



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
