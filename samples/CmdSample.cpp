//
//  CmdSample.cpp
//  CmdSample
//
//  Created by Sungwoo Bae on 6/15/17.
//

#include <CatenisApiClient.h>

#include <iostream>
#include <string>
#include <vector>

using std::cin;
using std::cout;
using std::endl;
using std::string;

int main(int argc, char* argv[])
{
    
    if (argc != 3)
    {
        std::cout << "Usage: CmdSample <device_id> <api_access_secret> \n";
        return 1;
    }
    
    ctn::CtnApiClient client(argv[1], argv[2], "catenis.io", "", "beta");
    
    cout << endl;
    cout << "Usage:" << endl;
    cout << "    log" << endl;
    cout << "    send <device_id>" << endl;
    cout << "    read <message_id>" << endl;
    cout << "    retrieve <message_id>" << endl;
    cout << "    list" << endl;
    cout << endl;
    
    while(true)
    {
        string method, device_id, message_id, message;
        string result;
        
        cout << "> ";
        cin >> method;
        
        if(method == "log")
        {
            cin.ignore();
            cout << "enter message: ";
            std::getline(cin, message);
            
            client.logMessage(message, result);
        }
        else if(method == "send")
        {
            cin >> device_id;
            cin.ignore();
            cout << "enter message: ";
            std::getline(cin, message);
            
            client.sendMessage(ctn::Device(device_id), message, result);
        }
        else if(method == "read")
        {
            cin >> message_id;
            cin.ignore();
            
            client.readMessage(message_id, result);
        }
        else if(method == "retrieve")
        {
            cin >> message_id;
            cin.ignore();
            
            client.retrieveMessageContainer(message_id, result);
        }
        else if(method == "list")
        {
            cin.ignore();
            
            client.listMessages(result);
        }
        else
        {
            cout << "incorrect method" << endl;
        }
        
        cout << endl << result << endl;
        cout << endl;
    }
    
    return 0;
}
