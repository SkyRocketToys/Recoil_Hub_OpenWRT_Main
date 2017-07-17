//tangible_event.h:

//----------------------------------------------------------------------------------------
//	Copyright © 2004 - 2017 Tangible Software Solutions Inc.
//	This class can be used by anyone provided that the copyright notice remains intact.
//
//	This class is used to convert C# events to C++.
//----------------------------------------------------------------------------------------
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

#ifndef _TANGIBLE_EVENT_H
#define _TANGIBLE_EVENT_H

template<typename T>
class TangibleEvent final
{
private:
	std::unordered_map<std::wstring, T> namedListeners;
public:
	void addListener(const std::wstring &methodName, T namedEventHandlerMethod)
	{
		if (namedListeners.find(methodName) == namedListeners.end())
			namedListeners[methodName] = namedEventHandlerMethod;
	}
	void removeListener(const std::wstring &methodName)
	{
		if (namedListeners.find(methodName) != namedListeners.end())
			namedListeners.erase(methodName);
	}

private:
	std::vector<T> anonymousListeners;
public:
	void addListener(T unnamedEventHandlerMethod)
	{
		anonymousListeners.push_back(unnamedEventHandlerMethod);
	}

	std::vector<T> listeners()
	{
		std::vector<T> allListeners;
		for (auto listener : namedListeners)
		{
			allListeners.push_back(listener.second);
		}
		allListeners.insert(allListeners.end(), anonymousListeners.begin(), anonymousListeners.end());
		return allListeners;
	}
};

#endif