#pragma once
#include <string>
#include <sstream>  // Для работы со stringstream
#include <iomanip>  // Для управления форматами
#include "utils.h"
#include "settings/SettingsStore.h"

extern SettingsStore settings;

// публикация своего описания
class EventDescriptor : public Event {
public:

    EventDescriptor() : Event("descriptor") {}

    void toString(std::string& msg) override {
        addParam(msg, "type", settings.getProductType());
        addParam(msg, "model", settings.getModel());
        addParam(msg, "t1_clbr", float2string(settings.getUserCalibration(), 1));
        addParam(msg, "t1_clbr_factory", float2string(settings.getFactoryCalibration(), 1));
        addParam(msg, "h1_max_power", float2string(settings.getHeaterMaxPower(), 2));

        addParam(msg, "fw_ver", std::to_string(settings.getFirmwareVersion())); // прошивка
        addParam(msg, "hw_ver", std::to_string(settings.getHardwareVersion())); // плата
        addParam(msg, "dp_ver", std::to_string(settings.getDisplayVersion())); // диспл
        
        addParam(msg, "dp_upd", std::to_string(settings.getDisplayFirmwareUpdateState())); // статус обновления дисплея (1 - не завершено, 0 - завершено)

        addParam(msg, "id", getDeviceId()); // Уник id esp

        //addParam(msg, "vol", std::to_string(settings.getDeviceVolume()));
        //addParam(msg, "pow", std::to_string(settings.getDevicePower()));
    }

    std::string getDeviceId() {
        uint64_t chipId = ESP.getEfuseMac();
        
        // Создаем строку для хранения hex-значения chipId (16 символов + null-терминатор)
        char chipIdHexStr[17];  // 16 символов для 64-битного hex + 1 для null-терминатора
        
        // Преобразуем chipId в hex строку
        snprintf(chipIdHexStr, sizeof(chipIdHexStr), "%016llx", chipId);

        // Возвращаем строку
        return std::string(chipIdHexStr);
    }

/*
    std::string getDeviceId() {
        uint64_t chipId = ESP.getEfuseMac();

        // Преобразуем chipId в hex строку (16b) с ведущими нулями
        std::stringstream ss;
        ss << std::hex << std::setw(16) << std::setfill('0') << chipId;

        return ss.str();
    }
   */ 
};


