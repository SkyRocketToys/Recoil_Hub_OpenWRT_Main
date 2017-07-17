#include <iostream>
#include <string>
#include <typeinfo>
#include <cstring>

int main()
{
    int clientID = 1;

    char* typeName = (char *)typeid(clientID).name();
    std::wstring wTypeName = std::wstring(typeName, typeName+strlen(typeName));

    std::wcout << L"Hello world No " + std::to_wstring(clientID) + L" I am not a number " << std::endl;

    std::cout << typeid(clientID).name() << std::endl;
}
