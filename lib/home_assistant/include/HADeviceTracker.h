#ifndef AHA_HADEVICETRACKER_H
#define AHA_HADEVICETRACKER_H

#include "BaseDeviceType.h"

#ifdef ARDUINOHA_DEVICE_TRACKER

class HADeviceTracker : public BaseDeviceType
{
public:
    /**
     * Initializes device tracker.
     *
     * @param uniqueId Unique ID of the tracker. Recommended characters: [a-z0-9\-_]
     * @param initialState Initial state of the sensor.
                           It will be published right after "config" message in order to update HA state.
     */
    HADeviceTracker(
        const char* uniqueId
    );

    /**
     * Publishes configuration of the sensor to the MQTT.
     */
    virtual void onMqttConnected() override;

    /**
     * Changes state of the sensor and publishes MQTT message.
     * Please note that if a new value is the same as previous one,
     * the MQTT message won't be published.
     *
     * @param state New state of the sensor.
     * @returns Returns true if MQTT message has been published successfully.
     */
    bool setState(bool state);

    /**
     * Returns last known state of the sensor.
     * If setState method wasn't called the initial value will be returned.
     */
    inline bool getState() const
        { return _currentState; }

private:
    bool publishState(bool state);
    uint16_t calculateSerializedLength(const char* serializedDevice) const override;
    bool writeSerializedData(const char* serializedDevice) const override;

    const char* _class;
    bool _currentState;
};

#endif
#endif
