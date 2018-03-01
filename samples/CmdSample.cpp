//
//  CmdSample.cpp
//  CmdSample
//
//  Created by Sungwoo Bae on 6/15/17.
//


#include <iostream>
#include <string>
#include <vector>

#include <CatenisApiException.h>
#include <CatenisApiClient.h>

using std::cin;
using std::cout;
using std::endl;
using std::string;

using namespace ctn;

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
        
        if (method == "log")
        {
            cin.ignore();
            cout << "enter message: ";
            std::getline(cin, message);
            
            try
            {
                LogMessageResult data;
                client.logMessage(data, message);
                std::cout << data.messageId << std::endl;
            }
            catch(CatenisAPIException *errObject)
            {
                std::cerr << errObject->getErrorDescription() << std::endl;
                delete errObject;
            }
            catch(...)
            {
                std::cerr << "Unknown error encountered: call to client.logMessage." << std::endl;
            }
        }
        else if (method == "send")
        {
            cin >> device_id;
            cin.ignore();
            cout << "enter message: ";
            std::getline(cin, message);
            
            try
            {
                SendMessageResult data;
                client.sendMessage(data, ctn::Device(device_id), message); 
                std::cout << data.messageId << std::endl;
            }
            catch(CatenisAPIException *errObject)
            {
                std::cerr << errObject->getErrorDescription() << std::endl;
                delete errObject;
            }
            catch(...)
            {
                std::cerr << "Unknown error encountered: call to client.sendMessage." << std::endl;
            }
        }
        else if (method == "read")
        {
            cin >> message_id;
            cin.ignore();
            
            try
            {
                ReadMessageResult data;
                client.readMessage(data, message_id);
                std::cout << data.message << std::endl;
            }
            catch(CatenisAPIException *errObject)
            {
                std::cerr << errObject->getErrorDescription() << std::endl;
                delete errObject;
            }
            catch(...)
            {
                std::cerr << "Unknown error encountered: call to client.readMessage." << std::endl;
            }
        }
        else if (method == "retrieve")
        {
            cin >> message_id;
            cin.ignore();
            
            try
            {
                RetrieveMessageContainerResult data;
                client.retrieveMessageContainer(data, message_id);
                std::cout << "txid :" << data.blockchain.txid << std::endl;
                std::string val = data.blockchain.isConfirmed ? "true" : "false";
                std::cout << "isConfirmed :" << val << std::endl;

                if (data.externalStorage != nullptr)
                {
                    std::map<std::string, std::string>::iterator it = (*data.externalStorage).begin();
                    for (; it != (*data.externalStorage).end(); it++)
                    {
                        cout << "Key : (" << it->first << ") Value : (" << it->second << ")" << std::endl;
                    }
                }
            }
            catch(CatenisAPIException *errObject)
            {
                std::cerr << errObject->getErrorDescription() << std::endl;
                delete errObject;
            }
            catch(...)
            {
                std::cerr << "Unknown error encountered: call to client.retrieveMessage." << std::endl;
            }
        }
        else if (method == "list")
        {
            cin.ignore();
            
            try
            {
                ListMessagesResult data;
                client.listMessages(data);

                std::list<std::shared_ptr<MessageDescription>>::iterator it = data.messageList.begin();
                for (; it != data.messageList.end(); it++)
                {
                     MessageDescription msgD = *(*it);
                     std::cout << "--------------------------------Message";
                     std::cout << "--------------------------------" << endl;
                     std::cout << "MessageId\t\t\t: " << msgD.messageId << endl;
                     std::cout << "Action\t\t\t\t: " << msgD.action << endl;
                     std::cout << "Read\t\t\t\t: " << msgD.read << endl;
                     std::cout << "Direction\t\t\t: " << msgD.direction << endl;
                     std::cout << "Date\t\t\t\t: " << msgD.date << endl;

                     if (msgD.from != nullptr)
                     {
                          std::cout << "FromDeviceId\t\t\t: " << (*msgD.from).deviceId << endl;
                          std::cout << "FromName\t\t\t: " << (*msgD.from).name << endl;
                          std::cout << "FromProdUniqueId\t\t: " << (*msgD.from).prodUniqueId << endl;
                     }
                     if (msgD.to != nullptr)
                     {
                          std::cout << "ToDeviceId\t\t\t: " << (*msgD.to).deviceId << endl;
                          std::cout << "ToName\t\t\t\t: " << (*msgD.to).name << endl;
                          std::cout << "ToProdUniqueId\t\t\t: " << (*msgD.to).prodUniqueId << endl;
                     }
                     std::cout << "ReadConfirmationEnabled\t\t: " << msgD.readConfirmationEnabled << endl;
                } 
                std::cout << "MsgCount\t\t\t: " << data.msgCount << std::endl;
                std::cout << "CountExceeded\t\t\t: " << data.countExceeded << std::endl;
            }
            catch(CatenisAPIException *errObject)
            {
                std::cerr << errObject->getErrorDescription() << std::endl;
                delete errObject;
            }
            catch(...)
            {
                std::cerr << "Unknown error encountered: call to client.listMessage." << std::endl;
            }
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
