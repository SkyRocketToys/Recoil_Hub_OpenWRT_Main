#include <iostream>
#include <boost/array.hpp>
#include <boost/locale/encoding_utf.hpp>
#include <boost/locale.hpp>

using boost::locale::conv::utf_to_utf;
using namespace std;

#define STRINGIFY(x) utf8_to_wstring(#x)


class GameEvents
{

public:

	enum eventID
	{
		clientID
	};

	GameEvents();
};

std::wstring utf8_to_wstring(const std::string& str)
{
	return utf_to_utf<wchar_t>(str.c_str(), str.c_str() + str.size());
}

std::string wstring_to_utf8(const std::wstring& str)
{
	return utf_to_utf<char>(str.c_str(), str.c_str() + str.size());
}


int main(){
  boost::array<int, 4> arr = {{1,2,3,4}};
  std::cout << "hi" << arr[0] << std::endl;
  wcout << STRINGIFY(GameEvents::eventID::clientID) << std::endl;

  std::pair<std::wstring, long> newEntry(STRINGIFY(GameEvents::eventID::clientID), 1);
  return 0;
}


