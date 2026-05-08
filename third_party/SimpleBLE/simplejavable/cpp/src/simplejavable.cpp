#include <jni.h>
#include <string>

#include "fmt/core.h"
#include <simpleble/Logging.h>
#include <simpleble/SimpleBLE.h>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

#include "core/AdapterWrapper.h"
#include "core/Cache.h"
#include "java/lang/ArrayList.h"
#include "java/lang/HashMap.h"
#include "java/lang/Integer.h"
#include "java/lang/Iterator.h"
#include "simplejni/Common.hpp"
#include "simplejni/Registry.hpp"
#include "org/simplejavable/AdapterCallback.h"
#include "org/simplejavable/Characteristic.h"
#include "org/simplejavable/DataCallback.h"
#include "org/simplejavable/Descriptor.h"
#include "org/simplejavable/PeripheralCallback.h"
#include "org/simplejavable/Service.h"

using namespace SimpleJNI;

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    SimpleJNI::Env env;
    try {
        SimpleJNI::Registrar::get().preload(env);
    } catch (const std::exception& e) {
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_org_simplejavable_Adapter_00024Companion_nativeIsBluetoothEnabled(JNIEnv* env, jobject thiz) {
    return SimpleBLE::Adapter::bluetooth_enabled();
}

extern "C" JNIEXPORT jlongArray JNICALL Java_org_simplejavable_Adapter_nativeGetAdapters(JNIEnv* env, jclass clazz) {
    std::vector<SimpleBLE::Adapter> adapters = SimpleBLE::Adapter::get_adapters();
    if (adapters.empty()) return env->NewLongArray(0);

    std::vector<int64_t> adapter_hashes;
    for (SimpleBLE::Adapter& adapter : adapters) {
        AdapterWrapper adapter_wrapper(adapter);
        adapter_hashes.push_back(adapter_wrapper.getHash());
        Cache::get().addAdapter(adapter_wrapper.getHash(), adapter_wrapper);
    }

    LongArray<ReleasableLocalRef> adapter_hashes_array(adapter_hashes);
    return adapter_hashes_array.release();
}

extern "C" JNIEXPORT void JNICALL Java_org_simplejavable_Adapter_nativeAdapterRegister(JNIEnv* env, jobject thiz,
                                                                                       jlong adapter_id,
                                                                                       jobject callback) {
    AdapterWrapper* adapter_wrapper = Cache::get().getAdapter(adapter_id);
    Org::SimpleJavaBLE::AdapterCallback adapter_callback(callback);
    adapter_wrapper->setCallback(adapter_callback);
}

extern "C" JNIEXPORT jstring JNICALL Java_org_simplejavable_Adapter_nativeAdapterIdentifier(JNIEnv* env, jobject thiz,
                                                                                            jlong adapter_id) {
    AdapterWrapper* adapter_wrapper = Cache::get().getAdapter(adapter_id);
    return String<ReleasableLocalRef>(adapter_wrapper->get().identifier()).release();
}

extern "C" JNIEXPORT jstring JNICALL Java_org_simplejavable_Adapter_nativeAdapterAddress(JNIEnv* env, jobject thiz,
                                                                                         jlong adapter_id) {
    AdapterWrapper* adapter_wrapper = Cache::get().getAdapter(adapter_id);
    return String<ReleasableLocalRef>(adapter_wrapper->get().address()).release();
}

extern "C" JNIEXPORT void JNICALL Java_org_simplejavable_Adapter_nativeAdapterScanStart(JNIEnv* env, jobject thiz,
                                                                                        jlong adapter_id) {
    AdapterWrapper* adapter_wrapper = Cache::get().getAdapter(adapter_id);
    adapter_wrapper->get().scan_start();
}

extern "C" JNIEXPORT void JNICALL Java_org_simplejavable_Adapter_nativeAdapterScanStop(JNIEnv* env, jobject thiz,
                                                                                       jlong adapter_id) {
    AdapterWrapper* adapter_wrapper = Cache::get().getAdapter(adapter_id);
    adapter_wrapper->get().scan_stop();
}

extern "C" JNIEXPORT void JNICALL Java_org_simplejavable_Adapter_nativeAdapterScanFor(JNIEnv* env, jobject thiz,
                                                                                      jlong adapter_id, jint timeout) {
    AdapterWrapper* adapter_wrapper = Cache::get().getAdapter(adapter_id);
    adapter_wrapper->get().scan_for(timeout);
}

extern "C" JNIEXPORT jboolean JNICALL Java_org_simplejavable_Adapter_nativeAdapterScanIsActive(JNIEnv* env,
                                                                                               jobject thiz,
                                                                                               jlong adapter_id) {
    AdapterWrapper* adapter_wrapper = Cache::get().getAdapter(adapter_id);
    return adapter_wrapper->get().scan_is_active();
}

extern "C" JNIEXPORT jlongArray JNICALL Java_org_simplejavable_Adapter_nativeAdapterScanGetResults(JNIEnv* env,
                                                                                                   jobject thiz,
                                                                                                   jlong adapter_id) {
    AdapterWrapper* adapter_wrapper = Cache::get().getAdapter(adapter_id);
    std::vector<SimpleBLE::Peripheral> peripherals = adapter_wrapper->get().scan_get_results();

    std::vector<int64_t> peripheral_hashes;
    for (SimpleBLE::Peripheral& peripheral : peripherals) {
        PeripheralWrapper peripheral_wrapper(peripheral);
        peripheral_hashes.push_back(peripheral_wrapper.getHash());
        Cache::get().addPeripheral(adapter_wrapper->getHash(), peripheral_wrapper.getHash(), peripheral_wrapper);
    }

    LongArray<ReleasableLocalRef> peripheral_hashes_array(peripheral_hashes);
    return peripheral_hashes_array.release();
}

extern "C" JNIEXPORT jlongArray JNICALL
Java_org_simplejavable_Adapter_nativeAdapterGetPairedPeripherals(JNIEnv* env, jobject thiz, jlong adapter_id) {
    AdapterWrapper* adapter_wrapper = Cache::get().getAdapter(adapter_id);
    std::vector<SimpleBLE::Peripheral> peripherals = adapter_wrapper->get().get_paired_peripherals();

    std::vector<int64_t> peripheral_hashes;
    for (SimpleBLE::Peripheral& peripheral : peripherals) {
        PeripheralWrapper peripheral_wrapper(peripheral);
        peripheral_hashes.push_back(peripheral_wrapper.getHash());
        Cache::get().addPeripheral(adapter_wrapper->getHash(), peripheral_wrapper.getHash(), peripheral_wrapper);
    }

    LongArray<ReleasableLocalRef> peripheral_hashes_array(peripheral_hashes);
    return peripheral_hashes_array.release();
}

// PERIPHERAL

extern "C" JNIEXPORT void JNICALL Java_org_simplejavable_Peripheral_nativePeripheralRegister(JNIEnv* env, jobject thiz,
                                                                                             jlong adapter_id,
                                                                                             jlong peripheral_id,
                                                                                             jobject callback) {
    PeripheralWrapper* peripheral_wrapper = Cache::get().getPeripheral(adapter_id, peripheral_id);
    Org::SimpleJavaBLE::PeripheralCallback peripheral_callback(callback);
    peripheral_wrapper->setCallback(peripheral_callback);
}

extern "C" JNIEXPORT jstring JNICALL Java_org_simplejavable_Peripheral_nativePeripheralIdentifier(JNIEnv* env,
                                                                                                  jobject thiz,
                                                                                                  jlong adapter_id,
                                                                                                  jlong peripheral_id) {
    PeripheralWrapper* peripheral_wrapper = Cache::get().getPeripheral(adapter_id, peripheral_id);
    return String<ReleasableLocalRef>(peripheral_wrapper->get().identifier()).release();
}

extern "C" JNIEXPORT jstring JNICALL Java_org_simplejavable_Peripheral_nativePeripheralAddress(JNIEnv* env,
                                                                                               jobject thiz,
                                                                                               jlong adapter_id,
                                                                                               jlong peripheral_id) {
    PeripheralWrapper* peripheral_wrapper = Cache::get().getPeripheral(adapter_id, peripheral_id);
    return String<ReleasableLocalRef>(peripheral_wrapper->get().address()).release();
}

extern "C" JNIEXPORT jint JNICALL Java_org_simplejavable_Peripheral_nativePeripheralAddressType(JNIEnv* env,
                                                                                                jobject thiz,
                                                                                                jlong adapter_id,
                                                                                                jlong peripheral_id) {
    PeripheralWrapper* peripheral_wrapper = Cache::get().getPeripheral(adapter_id, peripheral_id);
    return peripheral_wrapper->get().address_type();
}

extern "C" JNIEXPORT jint JNICALL Java_org_simplejavable_Peripheral_nativePeripheralRssi(JNIEnv* env, jobject thiz,
                                                                                         jlong adapter_id,
                                                                                         jlong peripheral_id) {
    PeripheralWrapper* peripheral_wrapper = Cache::get().getPeripheral(adapter_id, peripheral_id);
    return peripheral_wrapper->get().rssi();
}

extern "C" JNIEXPORT jint JNICALL Java_org_simplejavable_Peripheral_nativePeripheralTxPower(JNIEnv* env, jobject thiz,
                                                                                            jlong adapter_id,
                                                                                            jlong peripheral_id) {
    PeripheralWrapper* peripheral_wrapper = Cache::get().getPeripheral(adapter_id, peripheral_id);
    return peripheral_wrapper->get().tx_power();
}

extern "C" JNIEXPORT jint JNICALL Java_org_simplejavable_Peripheral_nativePeripheralMtu(JNIEnv* env, jobject thiz,
                                                                                        jlong adapter_id,
                                                                                        jlong peripheral_id) {
    PeripheralWrapper* peripheral_wrapper = Cache::get().getPeripheral(adapter_id, peripheral_id);
    return peripheral_wrapper->get().mtu();
}

extern "C" JNIEXPORT void JNICALL Java_org_simplejavable_Peripheral_nativePeripheralConnect(JNIEnv* env, jobject thiz,
                                                                                            jlong adapter_id,
                                                                                            jlong peripheral_id) {
    PeripheralWrapper* peripheral_wrapper = Cache::get().getPeripheral(adapter_id, peripheral_id);
    peripheral_wrapper->get().connect();
}

extern "C" JNIEXPORT void JNICALL Java_org_simplejavable_Peripheral_nativePeripheralDisconnect(JNIEnv* env,
                                                                                               jobject thiz,
                                                                                               jlong adapter_id,
                                                                                               jlong peripheral_id) {
    PeripheralWrapper* peripheral_wrapper = Cache::get().getPeripheral(adapter_id, peripheral_id);
    peripheral_wrapper->get().disconnect();
}

extern "C" JNIEXPORT void JNICALL Java_org_simplejavable_Peripheral_nativePeripheralNotify(
    JNIEnv* env, jobject thiz, jlong adapter_id, jlong peripheral_id, jstring j_service, jstring j_characteristic,
    jobject callback) {
    PeripheralWrapper* peripheral_wrapper = Cache::get().getPeripheral(adapter_id, peripheral_id);
    Org::SimpleJavaBLE::DataCallback data_callback(callback);
    String<LocalRef> service(j_service);
    String<LocalRef> characteristic(j_characteristic);
    peripheral_wrapper->get().notify(service.str(), characteristic.str(),
                                     [data_callback](SimpleBLE::ByteArray payload) {
                                         Org::SimpleJavaBLE::DataCallback data_callback_local(data_callback);
                                         ByteArray<ReleasableLocalRef> j_payload(payload);
                                         data_callback_local.on_data_received(j_payload.release());
                                     });
}

extern "C" JNIEXPORT void JNICALL Java_org_simplejavable_Peripheral_nativePeripheralIndicate(
    JNIEnv* env, jobject thiz, jlong adapter_id, jlong peripheral_id, jstring j_service, jstring j_characteristic,
    jobject callback) {
    PeripheralWrapper* peripheral_wrapper = Cache::get().getPeripheral(adapter_id, peripheral_id);
    Org::SimpleJavaBLE::DataCallback data_callback(callback);
    String<LocalRef> service(j_service);
    String<LocalRef> characteristic(j_characteristic);
    peripheral_wrapper->get().indicate(service.str(), characteristic.str(),
                                       [data_callback](SimpleBLE::ByteArray payload) {
                                           Org::SimpleJavaBLE::DataCallback data_callback_local(data_callback);
                                           ByteArray<ReleasableLocalRef> j_payload(payload);
                                           data_callback_local.on_data_received(j_payload.release());
                                       });
}
extern "C" JNIEXPORT void JNICALL Java_org_simplejavable_Peripheral_nativePeripheralUnsubscribe(
    JNIEnv* env, jobject thiz, jlong adapter_id, jlong peripheral_id, jstring j_service, jstring j_characteristic) {
    PeripheralWrapper* peripheral_wrapper = Cache::get().getPeripheral(adapter_id, peripheral_id);
    String<LocalRef> service(j_service);
    String<LocalRef> characteristic(j_characteristic);
    peripheral_wrapper->get().unsubscribe(service.str(), characteristic.str());
}

extern "C" JNIEXPORT jboolean JNICALL Java_org_simplejavable_Peripheral_nativePeripheralIsConnected(
    JNIEnv* env, jobject thiz, jlong adapter_id, jlong peripheral_id) {
    PeripheralWrapper* peripheral_wrapper = Cache::get().getPeripheral(adapter_id, peripheral_id);
    return peripheral_wrapper->get().is_connected();
}

extern "C" JNIEXPORT jboolean JNICALL Java_org_simplejavable_Peripheral_nativePeripheralIsConnectable(
    JNIEnv* env, jobject thiz, jlong adapter_id, jlong peripheral_id) {
    PeripheralWrapper* peripheral_wrapper = Cache::get().getPeripheral(adapter_id, peripheral_id);
    return peripheral_wrapper->get().is_connectable();
}

extern "C" JNIEXPORT jboolean JNICALL Java_org_simplejavable_Peripheral_nativePeripheralIsPaired(JNIEnv* env,
                                                                                                 jobject thiz,
                                                                                                 jlong adapter_id,
                                                                                                 jlong peripheral_id) {
    PeripheralWrapper* peripheral_wrapper = Cache::get().getPeripheral(adapter_id, peripheral_id);
    return peripheral_wrapper->get().is_paired();
}

extern "C" JNIEXPORT void JNICALL Java_org_simplejavable_Peripheral_nativePeripheralUnpair(JNIEnv* env, jobject thiz,
                                                                                           jlong adapter_id,
                                                                                           jlong peripheral_id) {
    PeripheralWrapper* peripheral_wrapper = Cache::get().getPeripheral(adapter_id, peripheral_id);
    peripheral_wrapper->get().unpair();
}

extern "C" JNIEXPORT jobject JNICALL Java_org_simplejavable_Peripheral_nativePeripheralServices(JNIEnv* env,
                                                                                                jobject thiz,
                                                                                                jlong adapter_id,
                                                                                                jlong peripheral_id) {
    PeripheralWrapper* peripheral_wrapper = Cache::get().getPeripheral(adapter_id, peripheral_id);

    Java::Util::ArrayList<ReleasableLocalRef> services_list;
    for (auto& service : peripheral_wrapper->get().services()) {
        Java::Util::ArrayList<ReleasableLocalRef> characteristics_list;
        for (auto& characteristic : service.characteristics()) {
            Java::Util::ArrayList<ReleasableLocalRef> descriptors_list;
            for (auto& descriptor : characteristic.descriptors()) {
                Org::SimpleJavaBLE::Descriptor descriptor_obj(descriptor.uuid());
                descriptors_list.add(descriptor_obj);
            }
            Org::SimpleJavaBLE::Characteristic characteristic_obj(
                characteristic.uuid(), descriptors_list.to_local(), characteristic.can_read(),
                characteristic.can_write_request(), characteristic.can_write_command(), characteristic.can_notify(),
                characteristic.can_indicate());
            characteristics_list.add(characteristic_obj);
        }
        Org::SimpleJavaBLE::Service service_obj(service.uuid(), characteristics_list.to_local());
        services_list.add(service_obj);
    }
    return services_list.release();
}

extern "C" JNIEXPORT jobject JNICALL Java_org_simplejavable_Peripheral_nativePeripheralManufacturerData(
    JNIEnv* env, jobject thiz, jlong adapter_id, jlong instance_id) {
    PeripheralWrapper* peripheral_wrapper = Cache::get().getPeripheral(adapter_id, instance_id);
    auto manufacturer_data = peripheral_wrapper->get().manufacturer_data();

    Java::Util::HashMap<ReleasableLocalRef> hash_map;
    for (const auto& [key, value] : manufacturer_data) {
        hash_map.put<LocalRef, LocalRef>(Java::Util::Integer<LocalRef>(key), ByteArray<LocalRef>(value));
    }
    return hash_map.release();
}

extern "C" JNIEXPORT jbyteArray JNICALL Java_org_simplejavable_Peripheral_nativePeripheralRead(
    JNIEnv* env, jobject thiz, jlong adapter_id, jlong peripheral_id, jstring j_service, jstring j_characteristic) {
    PeripheralWrapper* peripheral_wrapper = Cache::get().getPeripheral(adapter_id, peripheral_id);
    String<LocalRef> service(j_service);
    String<LocalRef> characteristic(j_characteristic);
    return ByteArray<ReleasableLocalRef>(peripheral_wrapper->get().read(service.str(), characteristic.str())).release();
}

extern "C" JNIEXPORT void JNICALL Java_org_simplejavable_Peripheral_nativePeripheralWriteRequest(
    JNIEnv* env, jobject thiz, jlong adapter_id, jlong peripheral_id, jstring j_service, jstring j_characteristic,
    jbyteArray j_data) {
    PeripheralWrapper* peripheral_wrapper = Cache::get().getPeripheral(adapter_id, peripheral_id);
    String<LocalRef> service(j_service);
    String<LocalRef> characteristic(j_characteristic);
    ByteArray<LocalRef> data(j_data);
    peripheral_wrapper->get().write_request(service.str(), characteristic.str(), data.bytes());
}

extern "C" JNIEXPORT void JNICALL Java_org_simplejavable_Peripheral_nativePeripheralWriteCommand(
    JNIEnv* env, jobject thiz, jlong adapter_id, jlong peripheral_id, jstring j_service, jstring j_characteristic,
    jbyteArray j_data) {
    PeripheralWrapper* peripheral_wrapper = Cache::get().getPeripheral(adapter_id, peripheral_id);
    String<LocalRef> service(j_service);
    String<LocalRef> characteristic(j_characteristic);
    ByteArray<LocalRef> data(j_data);
    peripheral_wrapper->get().write_command(service.str(), characteristic.str(), data.bytes());
}

extern "C" JNIEXPORT jbyteArray JNICALL Java_org_simplejavable_Peripheral_nativePeripheralDescriptorRead(
    JNIEnv* env, jobject thiz, jlong adapter_id, jlong peripheral_id, jstring j_service, jstring j_characteristic,
    jstring j_descriptor) {
    PeripheralWrapper* peripheral_wrapper = Cache::get().getPeripheral(adapter_id, peripheral_id);
    String<LocalRef> service(j_service);
    String<LocalRef> characteristic(j_characteristic);
    String<LocalRef> descriptor(j_descriptor);
    return ByteArray<ReleasableLocalRef>(
               peripheral_wrapper->get().read(service.str(), characteristic.str(), descriptor.str()))
        .release();
}

extern "C" JNIEXPORT void JNICALL Java_org_simplejavable_Peripheral_nativePeripheralDescriptorWrite(
    JNIEnv* env, jobject thiz, jlong adapter_id, jlong peripheral_id, jstring j_service, jstring j_characteristic,
    jstring j_descriptor, jbyteArray j_data) {
    PeripheralWrapper* peripheral_wrapper = Cache::get().getPeripheral(adapter_id, peripheral_id);
    String<LocalRef> service(j_service);
    String<LocalRef> characteristic(j_characteristic);
    String<LocalRef> descriptor(j_descriptor);
    ByteArray<LocalRef> data(j_data);
    peripheral_wrapper->get().write(service.str(), characteristic.str(), descriptor.str(), data.bytes());
}