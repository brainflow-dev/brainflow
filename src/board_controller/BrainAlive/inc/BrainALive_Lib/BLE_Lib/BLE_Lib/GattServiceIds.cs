using System.Collections;
using System.Collections.Generic;

namespace BLE_Lib
{
    public class GattServiceIds : IReadOnlyCollection<GattService>
    {
        #region Private Members

        /// <summary>
        /// The backing store for the list of services
        /// </summary>
        private readonly IReadOnlyCollection<GattService> mCollection;

        #endregion

        #region Public Properties

        /// <summary>
        /// The total number of items in the collection
        /// </summary>
        public int Count => mCollection.Count;

        #endregion

        #region Constructor

        /// <summary>
        /// Default constructor
        /// </summary>
        public GattServiceIds()
        {
            mCollection = new List<GattService>(new[]
            {
                new GattService("Generic Access", "org.bluetooth.service.generic_access", 0x1800, "GSS"),
                new GattService("Alert Notification Service", "org.bluetooth.service.alert_notification", 0x1811, "GSS"),
                new GattService("Automation IO", "org.bluetooth.service.automation_io", 0x1815, "GSS"),
                new GattService("Battery Service", "org.bluetooth.service.battery_service", 0x180F, "GSS"),
                new GattService("Binary Sensor", "GATT Service UUID", 0x183B, "BSS"),
                new GattService("Blood Pressure", "org.bluetooth.service.blood_pressure", 0x1810, "GSS"),
                new GattService("Body Composition", "org.bluetooth.service.body_composition", 0x181B, "GSS"),
                new GattService("Bond Management Service", "org.bluetooth.service.bond_management", 0x181E, "GSS"),
                new GattService("Continuous Glucose Monitoring", "org.bluetooth.service.continuous_glucose_monitoring", 0x181F, "GSS"),
                new GattService("Current Time Service", "org.bluetooth.service.current_time", 0x1805, "GSS"),
                new GattService("Cycling Power", "org.bluetooth.service.cycling_power", 0x1818, "GSS"),
                new GattService("Cycling Speed and Cadence", "org.bluetooth.service.cycling_speed_and_cadence", 0x1816, "GSS"),
                new GattService("Device Information", "org.bluetooth.service.device_information", 0x180A, "GSS"),
                new GattService("Emergency Configuration", "GATT Service UUID", 0x183C, "EMCS"),
                new GattService("Environmental Sensing", "org.bluetooth.service.environmental_sensing", 0x181A, "GSS"),
                new GattService("Fitness Machine", "org.bluetooth.service.fitness_machine", 0x1826, "GSS"),
                new GattService("Generic Attribute", "org.bluetooth.service.generic_attribute", 0x1801, "GSS"),
                new GattService("Glucose", "org.bluetooth.service.glucose", 0x1808, "GSS"),
                new GattService("Health Thermometer", "org.bluetooth.service.health_thermometer", 0x1809, "GSS"),
                new GattService("Heart Rate", "org.bluetooth.service.heart_rate", 0x180D, "GSS"),
                new GattService("HTTP Proxy", "org.bluetooth.service.http_proxy", 0x1823, "GSS"),
                new GattService("Human Interface Device", "org.bluetooth.service.human_interface_device", 0x1812, "GSS"),
                new GattService("Immediate Alert", "org.bluetooth.service.immediate_alert", 0x1802, "GSS"),
                new GattService("Indoor Positioning", "org.bluetooth.service.indoor_positioning", 0x1821, "GSS"),
                new GattService("Insulin Delivery", "org.bluetooth.service.insulin_delivery", 0x183A, "GSS"),
                new GattService("Internet Protocol Support Service", "org.bluetooth.service.internet_protocol_support", 0x1820, "GSS"),
                new GattService("Link Loss", "org.bluetooth.service.link_loss", 0x1803, "GSS"),
                new GattService("Location and Navigation", "org.bluetooth.service.location_and_navigation", 0x1819, "GSS"),
                new GattService("Mesh Provisioning Service", "org.bluetooth.service.mesh_provisioning", 0x1827, "GSS"),
                new GattService("Mesh Proxy Service", "org.bluetooth.service.mesh_proxy", 0x1828, "GSS"),
                new GattService("Next DST Change Service", "org.bluetooth.service.next_dst_change", 0x1807, "GSS"),
                new GattService("Object Transfer Service", "org.bluetooth.service.object_transfer", 0x1825, "GSS"),
                new GattService("Phone Alert Status Service", "org.bluetooth.service.phone_alert_status", 0x180E, "GSS"),
                new GattService("Pulse Oximeter Service", "org.bluetooth.service.pulse_oximeter", 0x1822, "GSS"),
                new GattService("Reconnection Configuration", "org.bluetooth.service.reconnection_configuration", 0x1829, "GSS"),
                new GattService("Reference Time Update Service", "org.bluetooth.service.reference_time_update", 0x1806, "GSS"),
                new GattService("Running Speed and Cadence", "org.bluetooth.service.running_speed_and_cadence", 0x1814, "GSS"),
                new GattService("Scan Parameters", "org.bluetooth.service.scan_parameters", 0x1813, "GSS"),
                new GattService("Transport Discovery", "org.bluetooth.service.transport_discovery", 0x1824, "GSS"),
                new GattService("Tx Power", "org.bluetooth.service.tx_power", 0x1804, "GSS"),
                new GattService("User Data", "org.bluetooth.service.user_data", 0x181C, "GSS"),
                new GattService("Weight Scale", "org.bluetooth.service.weight_scale", 0x181D, "GSS")
            });
        }

        #endregion

        #region IReadOnlyCollection Methods

        /// <summary>
        /// Gets the underlying enumerator of the collection
        /// </summary>
        /// <returns></returns>
        public IEnumerator<GattService> GetEnumerator() => mCollection.GetEnumerator();

        /// <summary>
        /// Gets the underlying enumerator of the collection
        /// </summary>
        /// <returns></returns>
        IEnumerator IEnumerable.GetEnumerator() => mCollection.GetEnumerator();

        #endregion
    }
}