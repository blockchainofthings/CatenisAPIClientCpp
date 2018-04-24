//
//  CmdSample.cpp
//  CmdSample
//
//  Created by Sungwoo Bae on 6/15/17.
//


#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include <CatenisApiException.h>
#include <CatenisApiClient.h>

using std::cin;
using std::cout;
using std::endl;
using std::string;

using namespace ctn;

void showUsage() {
    cout << endl << "Usage:" << endl;
    cout << "    msgOpts" << endl;
    cout << "    logMessage" << endl;
    cout << "    sendMessage <device_id> [<isProdUniqueId>]" << endl;
    cout << "    readMessage <message_id>" << endl;
    cout << "    retrieveMsgContainer <message_id>" << endl;
    cout << "    listMessages" << endl;
    cout << "    listPermissionEvents" << endl;
    cout << "    retrievePermissionRights <event_name>" << endl;
    cout << "    setPermissionRights <event_name>" << endl;
    cout << "    listNotificationEvents" << endl;
    cout << "    checkEffectivePermissionRight <event_name> <deviceId> [<isProdUniqueId>]" << endl;
    cout << "    retrieveDeviceIdInfo <deviceId> [<isProdUniqueId>]" << endl;
	cout << "    wsOpenNotificationChannel" << endl;
    cout << "    help" << endl;
    cout << "    exit" << endl;
}

void showMsgOptsUsage() {
    cout << endl << "Message options:" << endl;
    cout << "     encoding (utf8|base64|hex)" << endl;
    cout << "     encrypt (true|false)" << endl;
    cout << "     storage (auto|embedded|external)" << endl;
    cout << "     readConfirmation (true|false)" << endl;
    cout << "     show" << endl;
    cout << "     help" << endl;
    cout << "     exit" << endl;
}

void showMsgOptions(MessageOptions &msgOpts) {
    cout << "Encoding: " << msgOpts.encoding << endl;
    cout << "Encrypt: " << (msgOpts.encrypt ? "true" : "false") << endl;
    cout << "Storage: " << msgOpts.storage << endl;
    cout << "Read confirmation: " << (msgOpts.readConfirmation ? "true" : "false") << endl;
}

int main(int argc, char* argv[])
{
    
    if (argc != 3)
    {
        cout << "Usage: CmdSample <device_id> <api_access_secret> \n";
        return 1;
    }
    
    //ctn::CtnApiClient client(argv[1], argv[2], "catenis.io", "", "beta");
    ctn::CtnApiClient client(argv[1], argv[2], "localhost", "3000", "prod", false);

    showUsage();

    bool exit = false;
    MessageOptions msgOpts;

    do
    {
        string method, message_id, message;

        cout << endl << "> ";
        string cmdLine;
        std::getline(cin, cmdLine);
        std::istringstream isCmdLine(cmdLine);

        isCmdLine >> method;

        if (method == "msgOpts") {
            showMsgOptsUsage();

            bool optExit = false;

            do {
                string opt;

                cout << endl << "msgOpts> ";
                string msgOptsCmdLine;
                std::getline(cin, msgOptsCmdLine);
                std::istringstream isMsgOptsCmdLine(msgOptsCmdLine);
                isMsgOptsCmdLine >> opt;

                if (opt == "encoding") {
                    isMsgOptsCmdLine >> msgOpts.encoding;
                }
                else  if (opt == "encrypt") {
                    string boolVal;

                    isMsgOptsCmdLine >> boolVal;

                    msgOpts.encrypt = boolVal == "true";
                }
                else  if (opt == "storage") {
                    isMsgOptsCmdLine >> msgOpts.storage;
                }
                else if (opt == "exit") {
                    optExit = true;
                }
                else  if (opt == "readConfirmation") {
                    string boolVal;

                    isMsgOptsCmdLine >> boolVal;

                    msgOpts.readConfirmation = boolVal == "true";
                }
                else  if (opt == "show") {
                    showMsgOptions(msgOpts);
                }
                else  if (opt == "help") {
                    showMsgOptsUsage();
                }
                else {
                    cout << "Invalid command" << endl;
                }

                if (optExit) {
                    cout << endl << "Leaving message options setting" << endl;
                }
            }
            while (!optExit);
        }
        else if (method == "logMessage")
        {
            cout << "enter message: ";
            std::getline(cin, message);

            try
            {
                LogMessageResult data;
                client.logMessage(data, message, msgOpts);
                cout << "Returned message ID: " << data.messageId << std::endl;
            }
            catch(CatenisAPIException &errObject)
            {
                std::cerr << errObject.getErrorDescription() << std::endl;
            }
            catch(...)
            {
                std::cerr << "Unknown error encountered: call to client.logMessage." << std::endl;
            }
        }
        else if (method == "sendMessage")
        {
            string deviceId;
            bool isProdUniqueId = false;

            isCmdLine >> deviceId;

            if (!isCmdLine.eof()) {
                string strIsProdUniqueId;
                isCmdLine >> strIsProdUniqueId;
                isProdUniqueId = strIsProdUniqueId == "true";
            }

            cout << "enter message: ";
            std::getline(cin, message);

            try
            {
                SendMessageResult data;
                client.sendMessage(data, ctn::Device(deviceId, isProdUniqueId), message, msgOpts);
                cout << "Returned message ID: " << data.messageId << std::endl;
            }
            catch(CatenisAPIException &errObject)
            {
                std::cerr << errObject.getErrorDescription() << std::endl;
            }
            catch(...)
            {
                std::cerr << "Unknown error encountered: call to client.sendMessage." << std::endl;
            }
        }
        else if (method == "readMessage")
        {
            isCmdLine >> message_id;

            try
            {
                ReadMessageResult data;
                client.readMessage(data, message_id);
                cout << "Returned message info: " << std::endl;
                cout << "  Action: " << data.action << std::endl;
                cout << "  Message: " << data.message << std::endl;

                if (data.from != nullptr) {
                    cout << "  From: " << std::endl;
                    cout << "    Device ID: " << data.from->deviceId << std::endl;

                    if (!data.from->name.empty())
                        cout << "    Name: " << data.from->name << std::endl;

                    if (!data.from->prodUniqueId.empty())
                        cout << "    Product unique ID: " << data.from->prodUniqueId << std::endl;
                }
            }
            catch(CatenisAPIException &errObject)
            {
                std::cerr << errObject.getErrorDescription() << std::endl;
            }
            catch(...)
            {
                std::cerr << "Unknown error encountered: call to client.readMessage." << std::endl;
            }
        }
        else if (method == "retrieveMsgContainer")
        {
            isCmdLine >> message_id;

            try
            {
                RetrieveMessageContainerResult data;
                client.retrieveMessageContainer(data, message_id);
                cout << "Returned message container info:" << endl;
                cout << "  Blockchain txid: " << data.blockchain.txid << std::endl;
                string val = data.blockchain.isConfirmed ? "true" : "false";
                cout << "  Is confirmed: " << val << std::endl;

                if (data.externalStorage != nullptr)
                {
                    for (auto it = data.externalStorage->begin(); it != data.externalStorage->end(); it++)
                    {
                        cout << "  External storage (" << it->first << ") reference: " << it->second << std::endl;
                    }
                }
            }
            catch(CatenisAPIException &errObject)
            {
                std::cerr << errObject.getErrorDescription() << std::endl;
            }
            catch(...)
            {
                std::cerr << "Unknown error encountered: call to client.retrieveMessage." << std::endl;
            }
        }
        else if (method == "listMessages")
        {
            try
            {
                ListMessagesResult data;
                client.listMessages(data);

                for (auto it = data.messageList.begin(); it != data.messageList.end(); it++)
                {
                    MessageDescription msgD = *(*it);
                    cout << "--------------------------------Message";
                    cout << "--------------------------------" << endl;
                    cout << "MessageId\t\t\t: " << msgD.messageId << endl;
                    cout << "Action\t\t\t\t: " << msgD.action << endl;

                    if (!msgD.direction.empty())
                        cout << "Direction\t\t\t: " << msgD.direction << endl;

                    if (msgD.from != nullptr)
                    {
                        cout << "FromDeviceId\t\t\t: " << msgD.from->deviceId << endl;

                        if (!msgD.from->name.empty())
                            cout << "FromName\t\t\t: " << msgD.from->name << endl;

                        if (!msgD.from->prodUniqueId.empty())
                            cout << "FromProdUniqueId\t\t: " << msgD.from->prodUniqueId << endl;
                    }

                    if (msgD.to != nullptr)
                    {
                        cout << "ToDeviceId\t\t\t: " << msgD.to->deviceId << endl;

                        if (!msgD.to->name.empty())
                            cout << "ToName\t\t\t\t: " << msgD.to->name << endl;

                        if (!msgD.to->prodUniqueId.empty())
                            cout << "ToProdUniqueId\t\t\t: " << msgD.to->prodUniqueId << endl;
                    }

                    if (msgD.readConfirmationEnabled != nullptr)
                        cout << "ReadConfirmationEnabled\t\t: " << *msgD.readConfirmationEnabled << endl;

                    if (msgD.read != nullptr)
                        cout << "Read\t\t\t\t: " << *msgD.read << endl;

                    cout << "Date\t\t\t\t: " << msgD.date << endl;
                }
                cout << "MsgCount\t\t\t: " << data.msgCount << std::endl;
                cout << "CountExceeded\t\t\t: " << data.countExceeded << std::endl;
            }
            catch(CatenisAPIException &errObject)
            {
                std::cerr << errObject.getErrorDescription() << std::endl;
            }
            catch(...)
            {
                std::cerr << "Unknown error encountered: call to client.listMessage." << std::endl;
            }
        }
        else if (method == "listPermissionEvents")
        {
            try
            {
                ListPermissionEventsResult data;
                client.listPermissionEvents(data);

                for (auto it = data.permissionEvents.begin(); it != data.permissionEvents.end(); it++)
                {
                    cout << "Permission event (" << it->first << "): " << it->second << std::endl;
                }
            }
            catch (CatenisAPIException &errObject)
            {
                std::cerr << errObject.getErrorDescription() << std::endl;
            }
            catch (...)
            {
                std::cerr << "Unknown error encountered: call to client.listPermissionEvents." << std::endl;
            }
        }
        else if (method == "retrievePermissionRights")
        {
            string eventName;
            isCmdLine >> eventName;

            try
            {
                RetrievePermissionRightsResult data;
                client.retrievePermissionRights(data, eventName);

                /* SYSTEM LEVEL PERMISSION RIGHTS */
                cout << "System right: " << data.system << std::endl;

                /* CATENIS NODE PERMISSION RIGHTS */
                if (data.catenisNode != nullptr)
                {
                    cout << "\nCatenis node rights:" << std::endl;

                    std::list<string> allowed = data.catenisNode->allowed;

                    if (allowed.size() > 0) {
                        cout << "  Allowed Catenis nodes:" << endl;
                        for (auto i = allowed.begin(); i != allowed.end(); ++i)
                        {
                            cout << "    " << *i << endl;
                        }
                    }

                    std::list<string> denied = data.catenisNode->denied;
                    if (denied.size() > 0) {
                        cout << "  Denied Catenis nodes:" << endl;
                        for (auto i = denied.begin(); i != denied.end(); ++i)
                        {
                            cout << "    " << *i << endl;
                        }
                    }
                }

                /* CLIENT NODE PERMISSION RIGHTS */
                if (data.client != nullptr)
                {
                    cout << "\nClient rights:" << std::endl;

                    std::list<string> allowed = data.client->allowed;

                    if (allowed.size() > 0) {
                        cout << "  Allowed clients:" << endl;
                        for (auto i = allowed.begin(); i != allowed.end(); ++i)
                        {
                            cout << "    " << *i << endl;
                        }
                    }

                    std::list<string> denied = data.client->denied;
                    if (denied.size() > 0) {
                        cout << "  Denied clients:" << endl;
                        for (auto i = denied.begin(); i != denied.end(); ++i)
                        {
                            cout << "    " << *i << endl;
                        }
                    }
                }

                /* DEVICE LEVEL PERMISSION RIGHTS */
                if (data.device != nullptr)
                {
                    cout << "\nDevice rights:" << std::endl;

                    std::list<std::shared_ptr<DeviceInfo>> allowed = data.device->allowed;
                    if (allowed.size() > 0) {
                        cout << "  Allowed devices:" << endl;
                        int devCount = 0;
                        for (auto it = allowed.begin(); it != allowed.end(); it++)
                        {
                            cout << "    Device #" << ++devCount << ":" << endl;
                            DeviceInfo okDev = *(*it);
                            cout << "      deviceId: " << okDev.deviceId << endl;
                            if (!okDev.name.empty())
                                cout << "      name: " << okDev.name << endl;
                            if (!okDev.prodUniqueId.empty())
                                cout << "      prodUniqueId: " << okDev.prodUniqueId << endl;
                        }
                    }

                    std::list<std::shared_ptr<DeviceInfo>> denied = data.device->denied;
                    if (denied.size() > 0) {
                        cout << "  Denied devices:" << endl;
                        int devCount = 0;
                        for (auto it = denied.begin(); it != denied.end(); it++)
                        {
                            cout << "    Device #" << ++devCount << ":" << endl;
                            DeviceInfo ngDev = *(*it);
                            cout << "      deviceId: " << ngDev.deviceId << endl;
                            if (!ngDev.name.empty())
                                cout << "      name: " << ngDev.name << endl;
                            if (!ngDev.prodUniqueId.empty())
                                cout << "      prodUniqueId: " << ngDev.prodUniqueId << endl;
                        }
                    }
                }
            }
            catch (CatenisAPIException &errObject)
            {
                std::cerr << errObject.getErrorDescription() << std::endl;
            }
            catch (...)
            {
                std::cerr << "Unknown error encountered: call to client.retrivePermissionEvents." << std::endl;
            }
        }
        else if (method == "listNotificationEvents")
        {
            try
            {
                ListNotificationEventsResult data;
                client.listNotificationEvents(data);

                for (auto it = data.notificationEvents.begin(); it != data.notificationEvents.end(); it++)
                {
                    cout << "Notification event (" << it->first << "): " << it->second << std::endl;
                }
            }
            catch (CatenisAPIException &errObject)
            {
                std::cerr << errObject.getErrorDescription() << std::endl;
            }
            catch (...)
            {
                std::cerr << "Unknown error encountered: call to client.listNotificationEvents." << std::endl;
            }
        }
        else if (method == "checkEffectivePermissionRight")
        {
            string eventName, deviceId;
            bool isProdUniqueId = false;

            isCmdLine >> eventName >> deviceId;

            if (!isCmdLine.eof()) {
                string strIsProdUniqueId;
                isCmdLine >> strIsProdUniqueId;
                isProdUniqueId = strIsProdUniqueId == "true";
            }

            try
            {
                CheckEffectivePermissionRightResult data;
                client.checkEffectivePermissionRight(data,eventName, Device(deviceId, isProdUniqueId));

                cout << "Permission right for device (\"" << data.effectivePermissionRight.begin()->first << "\"): " << data.effectivePermissionRight.begin()->second << std::endl;
            }
            catch (CatenisAPIException &errObject)
            {
                std::cerr << errObject.getErrorDescription() << std::endl;
            }
            catch (...)
            {
                std::cerr << "Unknown error encountered: call to client.checkEffectivePermissionRight." << std::endl;
            }
        }
        else if (method == "retrieveDeviceIdInfo")
        {
            string deviceId;
            bool isProdUniqueId = false;

            isCmdLine >> deviceId;

            if (!isCmdLine.eof()) {
                string strIsProdUniqueId;
                isCmdLine >> strIsProdUniqueId;
                isProdUniqueId = strIsProdUniqueId == "true";
            }

            try
            {
                DeviceIdInfoResult data;
                client.retrieveDeviceIdInfo(data, Device(deviceId, isProdUniqueId));

                // Print out CatenisNodeInfo
                cout << "Catenis node:" << std::endl;
                cout << "  index: " << data.catenisNode->index << endl;

                if (!data.catenisNode->name.empty())
                    cout << "  name: " << data.catenisNode->name << endl;

                if (!data.catenisNode->description.empty())
                    cout << "  description: " << data.catenisNode->description << endl;

                // Print out ClientInfo
                cout << "\nClient:" << std::endl;
                cout << "  clientId: " << data.client->clientId << endl;

                if (!data.device->name.empty())
                    cout << "  name: " << data.client->name << endl;

                // Print out DeviceInfo
                cout << "\nDevice:" << endl;
                cout << "  deviceId: " << data.device->deviceId << endl;

                if (!data.device->name.empty())
                    cout << "  name: " << data.device->name << endl;

                if (!data.device->prodUniqueId.empty())
                    cout << "  prodUniqueId: " << data.device->prodUniqueId << endl;
            }
            catch (CatenisAPIException &errObject)
            {
                std::cerr << errObject.getErrorDescription() << std::endl;
            }
            catch (...)
            {
                std::cerr << "Unknown error encountered: call to client.retrieveDeviceIdInfo." << std::endl;
            }
        }
        else if (method == "setPermissionRights")
        {
            string eventName;
            isCmdLine >> eventName;

            // ------ SYSTEM LEVEL
            cout << "Enter system right (allow/deny): ";
            string systemRight;
            std::getline(cin, systemRight);

            // ------ CATENIS NODE LEVEL
            SetRightsCtnNode ctnNodeRights;

            cout << "Enter Catenis nodes (indices) to allow: ";
            string strAllowedCtnNodes;
            std::getline(cin, strAllowedCtnNodes);

            if (!strAllowedCtnNodes.empty()) {
                std::istringstream is(strAllowedCtnNodes);

                while (!is.eof()) {
                    string ctnNodeIndex;
                    is >> ctnNodeIndex;
                    ctnNodeRights.allowed.push_back(ctnNodeIndex);
                }
            }

            cout << "Enter Catenis nodes (indices) to deny: ";
            string strDeniedCtnNodes;
            std::getline(cin, strDeniedCtnNodes);

            if (!strDeniedCtnNodes.empty()) {
                std::istringstream is(strDeniedCtnNodes);

                while (!is.eof()) {
                    string ctnNodeIndex;
                    is >> ctnNodeIndex;
                    ctnNodeRights.denied.push_back(ctnNodeIndex);
                }
            }

            cout << "Enter Catenis nodes (indices) to clear right setting: ";
            string strNoneCtnNodes;
            std::getline(cin, strNoneCtnNodes);

            if (!strNoneCtnNodes.empty()) {
                std::istringstream is(strNoneCtnNodes);

                while (!is.eof()) {
                    string ctnNodeIndex;
                    is >> ctnNodeIndex;
                    ctnNodeRights.none.push_back(ctnNodeIndex);
                }
            }

            // ------ CLIENT LEVEL
            SetRightsClient clientRights;

            cout << "Enter clients (IDs) to allow: ";
            string strAllowedClients;
            std::getline(cin, strAllowedClients);

            if (!strAllowedClients.empty()) {
                std::istringstream is(strAllowedClients);

                while (!is.eof()) {
                    string clientId;
                    is >> clientId;
                    clientRights.allowed.push_back(clientId);
                }
            }

            cout << "Enter clients (IDs) to deny: ";
            string strDeniedClients;
            std::getline(cin, strDeniedClients);

            if (!strDeniedClients.empty()) {
                std::istringstream is(strDeniedClients);

                while (!is.eof()) {
                    string clientId;
                    is >> clientId;
                    clientRights.denied.push_back(clientId);
                }
            }

            cout << "Enter clients (IDs) to clear right setting: ";
            string strNoneClients;
            std::getline(cin, strNoneClients);

            if (!strNoneClients.empty()) {
                std::istringstream is(strNoneClients);

                while (!is.eof()) {
                    string clientId;
                    is >> clientId;
                    clientRights.none.push_back(clientId);
                }
            }

            // ------ DEVICE LEVEL
            SetRightsDevice deviceRights;

            cout << "Enter devices (IDs) to allow (precede product unique ID with %): ";
            string strAllowedDevices;
            std::getline(cin, strAllowedDevices);

            if (!strAllowedDevices.empty()) {
                std::istringstream is(strAllowedDevices);

                while (!is.eof()) {
                    string deviceId;
                    is >> deviceId;
                    bool isProdUniqueId = false;

                    if (deviceId.size() > 1 && deviceId.front() == '%') {
                        // Input ID is actually a product unique ID. Delete leading '%' character
                        deviceId.erase(0, 1);
                        isProdUniqueId = true;
                    }

                    deviceRights.allowed.push_back(Device(deviceId, isProdUniqueId));
                }
            }

            cout << "Enter devices (IDs) to deny (precede product unique ID with %): ";
            string strDeniedDevices;
            std::getline(cin, strDeniedDevices);

            if (!strDeniedDevices.empty()) {
                std::istringstream is(strDeniedDevices);

                while (!is.eof()) {
                    string deviceId;
                    is >> deviceId;
                    bool isProdUniqueId = false;

                    if (deviceId.size() > 1 && deviceId.front() == '%') {
                        // Input ID is actually a product unique ID. Delete leading '%' character
                        deviceId.erase(0, 1);
                        isProdUniqueId = true;
                    }

                    deviceRights.denied.push_back(Device(deviceId, isProdUniqueId));
                }
            }

            cout << "Enter devices (IDs) to clear right setting (precede product unique ID with %): ";
            string strNoneDevices;
            std::getline(cin, strNoneDevices);

            if (!strNoneDevices.empty()) {
                std::istringstream is(strNoneDevices);

                while (!is.eof()) {
                    string deviceId;
                    is >> deviceId;
                    bool isProdUniqueId = false;

                    if (deviceId.size() > 1 && deviceId.front() == '%') {
                        // Input ID is actually a product unique ID. Delete leading '%' character
                        deviceId.erase(0, 1);
                        isProdUniqueId = true;
                    }

                    deviceRights.none.push_back(Device(deviceId, isProdUniqueId));
                }
            }

            cout << endl;

            /*
            * Execute the Set Permission Right Command / and output the response
            */
            try
            {
                SetPermissionRightsResult data;
                client.setPermissionRights(data, eventName, systemRight, &ctnNodeRights, &clientRights, &deviceRights);
                cout << "Permission rights successfully set" << std::endl;
            }
            catch (CatenisAPIException &errObject)
            {
                std::cerr << errObject.getErrorDescription() << std::endl;
            }
            catch (...)
            {
                std::cerr << "Unknown error encountered: call to client.setPermissionRights." << std::endl;
            }
        }
        else if (method == "wsOpenNotificationChannel")
        {
            try
            {
                std::string data;
                std::string eventName = "receive-msg";
                std::string text = "This is a test";
                client.wsOpenNotificationChannel(data,eventName);
                cout << "Notification Channel Open Response (" << data << ") " << std::endl;
            }
            catch (CatenisAPIException &errObject)
            {
                std::cerr << errObject.getErrorDescription() << std::endl;
            }
            catch (...)
            {
                std::cerr << "Unknown error encountered: call to client.wsOpenNotificationChannel." << std::endl;
            }
        }
        else if (method == "help") {
            showUsage();
        }
        else if (method == "exit") {
            exit = true;
        }
        else
        {
            cout << "invalid command" << endl;
        }
    }
    while (!exit);
    
    return 0;
}
