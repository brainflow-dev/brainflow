#pragma once

#include "References.hpp"

#include <string>
#include <vector>
#include <unordered_map>


namespace SimpleJNI {

/**
 * @brief Descriptor for a single Java method to be preloaded.
 *
 * This struct holds the metadata for a method that a Java class needs to expose to C++.
 * It links a method name and signature to a jmethodID pointer where the resolved ID will
 * be stored during JNI_OnLoad.
 */
struct MethodDescriptor {
    std::string name;             ///< Name of the Java method (e.g., "startScan").
    std::string signature;        ///< JNI signature of the method (e.g., "(Landroid/bluetooth/le/ScanCallback;)V").
    jmethodID* target = nullptr;  ///< Pointer to where the resolved jmethodID will be stored.

    MethodDescriptor(const std::string& n, const std::string& sig, jmethodID* tgt)
        : name(n), signature(sig), target(tgt) {}
};

/**
 * @brief Descriptor for a Java class and its methods to be preloaded.
 *
 * This struct defines a Java class by its fully qualified name and a list of methods
 * that need JNI access. It ties the class and its methods to storage locations
 * (GlobalRef<jclass> and jmethodID pointers) that will be populated during preloading.
 */
struct JNIDescriptor {
    std::string class_name;  ///< Fully qualified Java class name (e.g., "android/bluetooth/le/BluetoothLeScanner").
    GlobalRef<jclass>* class_target;  ///< Pointer to where the resolved jclass will be stored.
    std::vector<MethodDescriptor> methods;       ///< List of methods to preload for this class.

    JNIDescriptor(const std::string& name, SimpleJNI::GlobalRef<jclass>* cls_target,
                  std::initializer_list<MethodDescriptor> meths)
        : class_name(name), class_target(cls_target), methods(meths) {}
};

/**
 * @brief Descriptor for a Java class and its static methods to be preloaded.
 *
 * This struct defines a Java class by its fully qualified name and a list of static methods
 * that need JNI access. It ties the class and its static methods to storage locations
 * (GlobalRef<jclass> and jmethodID pointers) that will be populated during preloading.
 */
struct StaticJNIDescriptor {
    std::string class_name;  ///< Fully qualified Java class name (e.g., "android/bluetooth/le/BluetoothLeScanner").
    GlobalRef<jclass>* class_target;  ///< Pointer to where the resolved jclass will be stored.
    std::vector<MethodDescriptor> static_methods;  ///< List of static methods to preload for this class.

    StaticJNIDescriptor(const std::string& name, SimpleJNI::GlobalRef<jclass>* cls_target,
                        std::initializer_list<MethodDescriptor> meths)
        : class_name(name), class_target(cls_target), static_methods(meths) {}
};

/**
 * @brief Singleton registrar for collecting and preloading JNI class descriptors.
 *
 * The Registrar collects JNIDescriptor and StaticJNIDescriptor instances at compile-time
 * (via static initialization) and preloads their JNI resources (jclass and jmethodID)
 * during JNI_OnLoad. This ensures all necessary JNI metadata is resolved early on the
 * correct thread, avoiding runtime ClassLoader or thread-context issues.
 */
class Registrar {
  public:
    /// @brief Get the singleton instance of the Registrar.
    static Registrar& get() {
        static Registrar instance;

        // Register common classes that are used in many places.
        static const JNIDescriptor desc{
            "java/lang/Object",  // Java class name
            nullptr,             // Where to store the jclass
            {                    // Instance methods to preload
                {"hashCode", "()I", nullptr},
            }
        };

        instance.register_descriptor(&desc);

        return instance;
    }

    /**
     * @brief Register a JNIDescriptor for preloading.
     *
     * Called automatically by AutoRegister during static initialization. Adds the descriptor
     * to an internal list that will be processed in preload().
     * @param descriptor Pointer to the JNIDescriptor to register.
     */
    void register_descriptor(const JNIDescriptor* descriptor) { descriptors.push_back(descriptor); }

    /**
     * @brief Register a StaticJNIDescriptor for preloading.
     *
     * Called automatically by AutoRegister during static initialization. Adds the descriptor
     * to an internal list that will be processed in preload().
     * @param descriptor Pointer to the StaticJNIDescriptor to register.
     */
    void register_static_descriptor(const StaticJNIDescriptor* descriptor) { static_descriptors.push_back(descriptor); }

    /**
     * @brief Preload all registered JNI resources.
     *
     * Called from JNI_OnLoad to resolve jclass and jmethodID for all registered descriptors.
     * Uses the provided JNIEnv to perform JNI lookups on the thread that loads the library,
     * ensuring access to the correct ClassLoader context (e.g., system classes on Android).
     * @param env JNIEnv pointer from JNI_OnLoad.
     * @throws std::runtime_error if a class or method cannot be resolved.
     */
    void preload(JNIEnv* env) {
        // Preload instance method descriptors
        for (const JNIDescriptor* desc : descriptors) {
            // Load the Java class
            jclass local_cls = env->FindClass(desc->class_name.c_str());
            if (!local_cls) {
                throw std::runtime_error("Failed to load class: " + desc->class_name);
            }

            GlobalRef<jclass> class_target(local_cls);
            env->DeleteLocalRef(local_cls);  // Clean up the local reference

            // If *desc->class_target is null, we don't set it.
            if (desc->class_target != nullptr) {
                *desc->class_target = class_target;
            }
            class_targets[desc->class_name] = class_target;

            // Load each method
            for (const MethodDescriptor& method : desc->methods) {
                jmethodID method_id = env->GetMethodID(class_target.get(), method.name.c_str(), method.signature.c_str());
                if (!method_id) {
                    throw std::runtime_error("Failed to get method: " + desc->class_name + "." + method.name);
                }

                // If *method.target is null, we don't set it.
                if (method.target != nullptr) {
                    *method.target = method_id;
                }
                method_targets[desc->class_name][method.name] = method_id;
            }
        }

        // Clear descriptors once they are preloaded
        descriptors.clear();

        // Preload static method descriptors
        for (const StaticJNIDescriptor* desc : static_descriptors) {
            // Load the Java class
            jclass local_cls = env->FindClass(desc->class_name.c_str());
            if (!local_cls) {
                throw std::runtime_error("Failed to load class: " + desc->class_name);
            }

            GlobalRef<jclass> class_target(local_cls);
            env->DeleteLocalRef(local_cls);  // Clean up the local reference

            // If *desc->class_target is null, we don't set it.
            if (desc->class_target != nullptr) {
                *desc->class_target = class_target;
            }
            static_class_targets[desc->class_name] = class_target;

            // Load each static method
            for (const MethodDescriptor& method : desc->static_methods) {
                jmethodID method_id = env->GetStaticMethodID(class_target.get(), method.name.c_str(), method.signature.c_str());
                if (!method_id) {
                    throw std::runtime_error("Failed to get static method: " + desc->class_name + "." + method.name);
                }

                // If *method.target is null, we don't set it.
                if (method.target != nullptr) {
                    *method.target = method_id;
                }
                static_method_targets[desc->class_name][method.name] = method_id;
            }
        }

        // Clear static descriptors once they are preloaded
        static_descriptors.clear();
    }

    GlobalRef<jclass> get_class(const std::string& name) {
        return class_targets[name];
    }

    jmethodID get_method(const std::string& class_name, const std::string& method_name) {
        return method_targets[class_name][method_name];
    }

    GlobalRef<jclass> get_static_class(const std::string& name) {
        return static_class_targets[name];
    }

    jmethodID get_static_method(const std::string& class_name, const std::string& method_name) {
        return static_method_targets[class_name][method_name];
    }

  private:
    Registrar() = default;
    ~Registrar() = default;
    Registrar(const Registrar&) = delete;
    Registrar& operator=(const Registrar&) = delete;

    std::unordered_map<std::string, GlobalRef<jclass>> class_targets;
    std::unordered_map<std::string, std::unordered_map<std::string, jmethodID>> method_targets;

    std::unordered_map<std::string, GlobalRef<jclass>> static_class_targets;
    std::unordered_map<std::string, std::unordered_map<std::string, jmethodID>> static_method_targets;

    std::vector<const JNIDescriptor*> descriptors;  ///< List of registered descriptors.
    std::vector<const StaticJNIDescriptor*> static_descriptors;  ///< List of registered static descriptors.
};

/**
 * @brief Helper to automatically register JNI descriptors at compile time.
 *
 * This template class uses static initialization to register descriptors with the Registrar
 * when the program loads. The constructor runs before main() or JNI_OnLoad, ensuring all
 * classes are registered without manual intervention. It supports registering non-static methods,
 * static methods, or both.
 * @tparam T The class type (e.g., BluetoothScanner) that owns the descriptor.
 */
template <typename T>
struct AutoRegister {
    // Constructor for non-static methods only
    AutoRegister(const JNIDescriptor* desc) {
        Registrar::get().register_descriptor(desc);
    }

    // Constructor for static methods only
    AutoRegister(const StaticJNIDescriptor* static_desc) {
        Registrar::get().register_static_descriptor(static_desc);
    }

    // Constructor for both non-static and static methods
    AutoRegister(const JNIDescriptor* desc, const StaticJNIDescriptor* static_desc) {
        Registrar::get().register_descriptor(desc);
        Registrar::get().register_static_descriptor(static_desc);
    }
};

}  // namespace SimpleJNI