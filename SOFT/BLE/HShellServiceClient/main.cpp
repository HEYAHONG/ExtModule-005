#include "hbox.h"
#include "stdio.h"
#include "string.h"
#include <string>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <map>
#include "simpleble/SimpleBLE.h"
#include "simpleble/PeripheralSafe.h"
#include "simpleble/Utils.h"
#include "simpleble/Logging.h"


static std::string DeviceName="EM005PRPH";
static std::string ServiceName="HShell";
static std::string CharacteristicName="HShell.IO";
int main(int argc,const char *argv[])
{
    setbuf(stdout,NULL);
    {
        printf("device name:%s\r\n",DeviceName.c_str());
    }
    huuid_t service_uuid;
    {
        huuid_md5_uuid_generate(service_uuid,(const uint8_t *)ServiceName.c_str(),ServiceName.length(),huuid_oid_uuid);
        huuid_string_t uuid_string;
        huuid_unparse(uuid_string,service_uuid);
        printf("service uuid:%s\r\n",uuid_string);
    }
    huuid_t characteristic_uuid;
    {
        huuid_md5_uuid_generate( characteristic_uuid,(const uint8_t *) CharacteristicName.c_str(), CharacteristicName.length(),huuid_oid_uuid);
        huuid_string_t uuid_string;
        huuid_unparse(uuid_string, characteristic_uuid);
        printf("characteristic uuid:%s\r\n",uuid_string);
    }
    SimpleBLE::Logging::Logger::get()->set_level(SimpleBLE::Logging::None);
    while (true)
    {
        try
        {
            auto adapter_list = SimpleBLE::Adapter::get_adapters();

            if (adapter_list.size() == 0)
            {
                printf("can not find adapter!\r\n");
                break;
            }

            size_t adapter_index = 0;

            while (adapter_list.size() > 1)
            {
                for (size_t i = 0; i < adapter_list.size(); i++)
                {
                    printf("\t%d\t%s\r\n", (int)i, adapter_list.at(i).identifier().c_str());
                }
                printf("input selected:");
                scanf("%d", (int*)&adapter_index);
                if (adapter_index < adapter_list.size())
                {
                    break;
                }
            }

            auto adapter = adapter_list.at(adapter_index);
            printf("apadter:%s(%s)\r\n", adapter.identifier().c_str(), adapter.address().c_str());
            std::map<std::string, SimpleBLE::Peripheral> peripheral_list;
            auto scan_callback = [&](SimpleBLE::Peripheral peripheral)
            {
                if (peripheral.identifier() == DeviceName)
                {
                    if (peripheral_list.find(peripheral.address()) == peripheral_list.end())
                    {
                        peripheral_list[peripheral.address()] = peripheral;
                        printf("peripheral:%s\r\n", peripheral.address().c_str());
                    }
                }
            };
            adapter.set_callback_on_scan_updated(scan_callback);
            adapter.set_callback_on_scan_found(scan_callback);
            adapter.scan_start();
            while (adapter.bluetooth_enabled())
            {

                std::this_thread::sleep_for(std::chrono::seconds(3));
                if (peripheral_list.size() > 0)
                {
                    adapter.scan_stop();
                    size_t peripheral_index = 0;
                    while (peripheral_list.size() > 1)
                    {
                        for (size_t i = 0; i < peripheral_list.size(); i++)
                        {
                            auto it = peripheral_list.begin();
                            std::advance(it, i);
                            printf("\t%d\t%s\r\n", (int)i, it->first.c_str());
                        }
                        printf("input selected:");
                        scanf("%d", (int*)&peripheral_index);
                        if (peripheral_index < peripheral_list.size())
                        {
                            break;
                        }
                    }

                    auto peripheral_pair= peripheral_list.begin();
                    std::advance(peripheral_pair, peripheral_index);

                    auto peripheral = SimpleBLE::Safe::Peripheral(peripheral_pair->second);
                    if (peripheral.is_connectable())
                    {
                        bool connected = false;
                        peripheral.set_callback_on_connected([&]()
                        {
                            connected = true;
                            printf("connected!\r\n");
                        });
                        peripheral.set_callback_on_disconnected([&]()
                        {
                            connected = false;
                            printf("disconnected!\r\n");
                        });
                        while (!connected)
                        {
                            std::this_thread::sleep_for(std::chrono::seconds(1));
                            try
                            {
                                peripheral.connect();
                            }
                            catch (...)
                            {

                            }
                            std::this_thread::sleep_for(std::chrono::seconds(1));

                        }
                        if (peripheral.is_connected())
                        {
                            while (true)
                            {
                                auto services = peripheral.services();
                                if (!services.has_value())
                                {
                                    printf("no service!\r\n");
                                    break;
                                }

                                for (auto service : (*services))
                                {
                                    huuid_t current_service_uuid;
                                    huuid_parse(current_service_uuid, service.uuid().c_str());
                                    if (huuid_compare(service_uuid, current_service_uuid) == 0)
                                    {
                                        printf("service found!\r\n");
                                        for (auto characteristic : service.characteristics())
                                        {
                                            huuid_t current_characteristic_uuid;
                                            huuid_parse(current_characteristic_uuid, characteristic.uuid().c_str());
                                            if (huuid_compare(current_characteristic_uuid, characteristic_uuid) == 0)
                                            {
                                                printf("characteristic found!\r\n");
                                                if (characteristic.can_notify())
                                                {
                                                    auto notify = [](SimpleBLE::ByteArray payload)
                                                    {
                                                        printf("%s",((std::string)payload).c_str());
                                                    };
                                                    if (peripheral.notify(service.uuid(), characteristic.uuid(),notify))
                                                    {
                                                        printf("notify ok!\r\n");
                                                        printf("now you can input:\r\n");
                                                        while (connected)
                                                        {
                                                            SimpleBLE::ByteArray payload;
                                                            int ch = 0;
                                                            while (true)
                                                            {
                                                                ch = getchar();
                                                                if (ch == EOF)
                                                                {
                                                                    break;
                                                                }
                                                                else
                                                                {
                                                                    payload.push_back((uint8_t)ch);
                                                                }
                                                                if (ch == '\n')
                                                                {
                                                                    break;
                                                                }
                                                            }
                                                            if (ch == EOF)
                                                            {
                                                                break;
                                                            }
                                                            if (characteristic.can_write_request())
                                                            {
                                                                peripheral.write_request(service.uuid(), characteristic.uuid(), payload);
                                                            }
                                                        }
                                                    }
                                                }
                                                break;
                                            }
                                        }
                                        break;
                                    }
                                }

                                break;
                            }
                        }
                    }
                }
            }
        }
        catch (...)
        {

        }

    }
    return 0;
}

