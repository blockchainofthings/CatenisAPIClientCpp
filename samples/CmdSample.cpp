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
    cout << "    msgOpts" << endl;
    cout << "    log" << endl;
    cout << "    send <device_id>" << endl;
    cout << "    read <message_id>" << endl;
    cout << "    retrieve <message_id>" << endl;
    cout << "    list" << endl;
	cout << "    listPermissionEvents" << endl;
	cout << "    retrievePermissionRights <event_name>" << endl;
	cout << "    listNotificationEvents" << endl;
	cout << "    checkEffectivePermissionRight" << endl;
    cout << "    exit" << endl;

    bool exit = false;
    MessageOptions msgOpts;

    do
    {
        string method, device_id, message_id, message;

        cout << endl << "> ";
        cin >> method;

        if (method == "msgOpts") {
            cin.ignore();

            cout << "Message options:" << endl;
            cout << "     encoding <msg_encoding>" << endl;
            cout << "     encrypt <bool>" << endl;
            cout << "     storage <msg_storage>" << endl;
            cout << "     readConfirmation <bool>" << endl;
            cout << "     exit" << endl;

            bool optExit = false;

            do {
                string opt;

                cout << endl << "msgOpts> ";
                cin >> opt;

                if (opt == "encoding") {
                    cin >> msgOpts.encoding;
                    cin.ignore();
                }
                else  if (opt == "encrypt") {
                    string boolVal;

                    cin >> boolVal;
                    cin.ignore();

                    msgOpts.encrypt = boolVal == "yes";
                }
                else  if (opt == "storage") {
                    cin >> msgOpts.storage;
                    cin.ignore();
                }
                else if (opt == "exit") {
                    optExit = true;
                }
                else  if (opt == "readConfirmation") {
                    string boolVal;

                    cin >> boolVal;
                    cin.ignore();

                    msgOpts.readConfirmation = boolVal == "yes";
                }
                else {
                    cout << "Invalid message option" << endl;
                }

                if (optExit) {
                    cout << endl << "Leaving message options setting" << endl;
                }
            }
            while (!optExit);
        }
        else if (method == "log")
        {
            cin.ignore();
            cout << "enter message: ";
            std::getline(cin, message);

            try
            {
                LogMessageResult data;
                client.logMessage(data, message, msgOpts);
                std::cout << "Returned message ID: " << data.messageId << std::endl;
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
        else if (method == "send")
        {
            cin >> device_id;
            cin.ignore();
            cout << "enter message: ";
            std::getline(cin, message);

            try
            {
                SendMessageResult data;
                client.sendMessage(data, ctn::Device(device_id), message, msgOpts);
                std::cout << "Returned message ID: " << data.messageId << std::endl;
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
        else if (method == "read")
        {
            cin >> message_id;
            cin.ignore();

            try
            {
                ReadMessageResult data;
                client.readMessage(data, message_id);
                std::cout << "Returned message info: " << std::endl;
                std::cout << "  Action: " << data.action << std::endl;
                std::cout << "  Message: " << data.message << std::endl;

                if (data.from != nullptr) {
                    std::cout << "  From: " << std::endl;
                    std::cout << "    Device ID: " << data.from->deviceId << std::endl;

                    if (!data.from->name.empty())
                        std::cout << "    Name: " << data.from->name << std::endl;

                    if (!data.from->prodUniqueId.empty())
                        std::cout << "    Product unique ID: " << data.from->prodUniqueId << std::endl;
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
        else if (method == "retrieve")
        {
            cin >> message_id;
            cin.ignore();

            try
            {
                RetrieveMessageContainerResult data;
                client.retrieveMessageContainer(data, message_id);
                std::cout << "Returned message container info:" << endl;
                std::cout << "  Blockchain txid: " << data.blockchain.txid << std::endl;
                std::string val = data.blockchain.isConfirmed ? "true" : "false";
                std::cout << "  Is confirmed: " << val << std::endl;

                if (data.externalStorage != nullptr)
                {
                    std::map<std::string, std::string>::iterator it = data.externalStorage->begin();
                    for (; it != data.externalStorage->end(); it++)
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

                    if (!msgD.direction.empty())
                        std::cout << "Direction\t\t\t: " << msgD.direction << endl;

                    if (msgD.from != nullptr)
                    {
                        std::cout << "FromDeviceId\t\t\t: " << msgD.from->deviceId << endl;

                        if (!msgD.from->name.empty())
                            std::cout << "FromName\t\t\t: " << msgD.from->name << endl;

                        if (!msgD.from->prodUniqueId.empty())
                            std::cout << "FromProdUniqueId\t\t: " << msgD.from->prodUniqueId << endl;
                    }

                    if (msgD.to != nullptr)
                    {
                        std::cout << "ToDeviceId\t\t\t: " << msgD.to->deviceId << endl;

                        if (!msgD.to->name.empty())
                            std::cout << "ToName\t\t\t\t: " << msgD.to->name << endl;

                        if (!msgD.to->prodUniqueId.empty())
                            std::cout << "ToProdUniqueId\t\t\t: " << msgD.to->prodUniqueId << endl;
                    }

                    if (msgD.readConfirmationEnabled != nullptr)
                        std::cout << "ReadConfirmationEnabled\t\t: " << *msgD.readConfirmationEnabled << endl;

                    if (msgD.read != nullptr)
                        std::cout << "Read\t\t\t\t: " << *msgD.read << endl;

                    std::cout << "Date\t\t\t\t: " << msgD.date << endl;
                }
                std::cout << "MsgCount\t\t\t: " << data.msgCount << std::endl;
                std::cout << "CountExceeded\t\t\t: " << data.countExceeded << std::endl;
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
			cin.ignore();

			try
			{
				ListPermissionEventsResult data;
				client.listPermissionEvents(data);

                for (PermissionEventDictionary::iterator it = data.permissionEvents.begin(); it != data.permissionEvents.end(); it++)
                {
                    cout << "  Permission event (" << it->first << "): " << it->second << std::endl;
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
			cin >> eventName;
			cin.ignore();

			try
			{
				RetrievePermissionRightsResult data;
				client.retrievePermissionRights(data, eventName);

				/* SYSTEM LEVEL PERMISSION RIGHTS */
				std::cout << "\nSYSTEM:\t\t\t" << data.system << std::endl;

				/* CATENIS NODE PERMISSION RIGHTS */
				if (data.catenisNode != nullptr)
				{
					std::list<std::string> allowed = data.client->allowed;

					if (allowed.size() > 0) {
						std::cout << "" << std::endl;
						std::cout << "CATENIS NODES Allowed: " << std::endl;
						for (std::list<std::string>::const_iterator i = allowed.begin(); i != allowed.end(); ++i)
						{
							std::cout << "\t\t\t\t" + *i << std::endl;
						}
					}

					std::list<std::string> denied = data.client->denied;
					if (denied.size() > 0) {
						std::cout << "" << std::endl;
						std::cout << "CATENIS NODES Denied: " << std::endl;
						for (std::list<std::string>::const_iterator i = denied.begin(); i != denied.end(); ++i)
						{
							std::cout << "\t\t\t\t" + *i << std::endl;
						}
					}
				}

				/* CLIENT NODE PERMISSION RIGHTS */
				if (data.client != nullptr)
				{
					std::list<std::string> allowed = data.client->allowed;

					if (allowed.size() > 0) {
						std::cout << "" << std::endl;
						std::cout << "CLIENTS Allowed: " << std::endl;
						std::cout << "----------------------------------------------" << std::endl;
						for (std::list<std::string>::const_iterator i = allowed.begin(); i != allowed.end(); ++i)
						{
							std::cout << "\t\t" + *i << std::endl;
						}
					}

					std::list<std::string> denied = data.client->denied;
					if (denied.size() > 0) {
						std::cout << "" << std::endl;
						std::cout << "CLIENTS Denied: " << std::endl;
						std::cout << "----------------------------------------------" << std::endl;
						for (std::list<std::string>::const_iterator i = denied.begin(); i != denied.end(); ++i)
						{
							std::cout << "\t\t\t" + *i << std::endl;
						}
					}

					/* DEVICE LEVEL PERMISSION RIGHTS */
					if (data.device != nullptr)
					{
						std::cout << "" << std::endl;
						std::cout << "DEVICES Allowed: " << std::endl;
						std::cout << "----------------------------------------------" << std::endl;
						std::list<std::shared_ptr<DeviceInfo>> allowed = data.device->allowed;

						if (allowed.size() > 0) {
							std::list<std::shared_ptr<DeviceInfo>>::iterator it = allowed.begin();
							for (; it != allowed.end(); it++)
							{
								DeviceInfo okDev = *(*it);
								std::cout << "FromDeviceId\t\t: " << okDev.deviceId << endl;
								if (!okDev.name.empty())
									std::cout << "FromName\t\t: " << okDev.name << endl;
								if (!okDev.prodUniqueId.empty())
									std::cout << "FromProdUniqueId\t: " << okDev.prodUniqueId << endl;
								std::cout << "" << std::endl;
							}
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
			cin.ignore();

			try
			{
				ListNotificationEventsResult data;
				client.listNotificationEvents(data);

				for (NotificationEventDictionary::iterator it = data.notificationEvents.begin(); it != data.notificationEvents.end(); it++)
				{
					cout << "  Notification event (" << it->first << "): " << it->second << std::endl;
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
			string eventName, deviceId, isProdUniqueId = "false";

			cin >> eventName >> deviceId;

			if (deviceId != "self") {
				std::cout << "Is this a unique product ID?  (y or n)" << std::endl;
				cin >> isProdUniqueId;
			}
			cin.ignore();

			// Provide an option to set the "Product Unique ID" flag
			if (isProdUniqueId == "y") {
				isProdUniqueId = "true";

			try
			{
				CheckEffectivePermissionRightResult data;
				client.checkEffectivePermissionRight(data,eventName, deviceId, isProdUniqueId);

				for (EffectivePermissionRightDictionary::iterator it = data.effectivePermissionRight.begin(); it != data.effectivePermissionRight.end(); it++)
				{
					std::cout << "" << std::endl;
					std::cout << "Effective Permission Right for ( " << it->first << " ) is >> \t" << it->second << std::endl;
				}
				
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
        else if (method == "exit") {
            exit = true;
        }
        else
        {
            cout << "incorrect method" << endl;
        }
    }
    while (!exit);
    
    return 0;
}
