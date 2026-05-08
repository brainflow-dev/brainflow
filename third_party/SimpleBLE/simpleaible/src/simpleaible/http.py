try:
    from fastapi import FastAPI, HTTPException
    from pydantic import BaseModel
    import uvicorn
except ImportError:
    print("Dependencies for SimpleAIBLE HTTP server are not installed.")
    print("Please install them using: pip install simpleaible")
    exit(1)

from contextlib import asynccontextmanager
import simplepyble
from typing import List, Dict
import argparse

@asynccontextmanager
async def lifespan(app: FastAPI):
    # Startup logic
    print(f"Adapters found: {[a.identifier() for a in ble_state.adapters]}")
    if not ble_state.adapter :
        print("No bluetooth adapter found!")
    yield
    # Shutdown logic (none needed for now)

app = FastAPI(
    title="SimpleAIBLE API", 
    description="REST API to control BLE devices using SimplePyBLE", 
    version=simplepyble.get_simpleble_version(),
    lifespan=lifespan
)

# Global state to hold adapters and peripherals
# This might need more robust state management
class BleState:
    def __init__(self):
        self.adapters = simplepyble.Adapter.get_adapters()
        self.adapter = self.adapters[0] if self.adapters else None
        self.peripherals: Dict[str, simplepyble.Peripheral] = {}
        self.scan_results: List[simplepyble.Peripheral] = []
        # Store notifications: address -> list of {service, char, data_hex}
        self.notifications: Dict[str, List[Dict]] = {}

    def refresh_adapters(self):
        self.adapters = simplepyble.Adapter.get_adapters()
        if self.adapters and not self.adapter:
            self.adapter = self.adapters[0]

ble_state = BleState()

class DeviceInfo(BaseModel):
    identifier: str
    address: str
    is_connected: bool
    rssi: int
    mtu: int

class ServiceInfo(BaseModel):
    uuid: str
    characteristics: List[str]


@app.get("/")
def root():
    return {"message": "SimpleAIBLE API is running"}

@app.get("/adapters")
def get_adapters():
    ble_state.refresh_adapters()
    return [{"identifier": a.identifier(), "address": a.address()} for a in ble_state.adapters]

@app.post("/scan")
def scan(timeout_ms: int = 5000):
    if not ble_state.adapter:
        raise HTTPException(status_code=500, detail="No Bluetooth adapter available")
    
    ble_state.adapter.scan_for(timeout_ms)
    ble_state.scan_results = ble_state.adapter.scan_get_results()
    
    results = []
    for p in ble_state.scan_results:
        man_data = {}
        for company_id, data in p.manufacturer_data().items():
            man_data[str(company_id)] = data.hex()

        results.append({
            "identifier": p.identifier(),
            "address": p.address(),
            "rssi": p.rssi(),
            "connectable": p.is_connectable(),
            "manufacturer_data": man_data
        })
    return results

@app.post("/connect/{address}")
def connect(address: str):
    # Check if already connected
    if address in ble_state.peripherals:
        p = ble_state.peripherals[address]
        if p.is_connected():
            return {"message": f"Already connected to {p.identifier()}"}
    
    # Find in scan results
    target = None
    for p in ble_state.scan_results:
        if p.address() == address:
            target = p
            break
            
    if not target:
        # Try to connect even if not in recent scan if we have reference? 
        # For now require scan first or if it's already in our peripherals list
        if address in ble_state.peripherals:
            target = ble_state.peripherals[address]
        else:
            raise HTTPException(status_code=404, detail="Device not found in scan results. Please scan first.")

    try:
        target.connect()
        ble_state.peripherals[address] = target
        # Initialize notification list for this device
        if address not in ble_state.notifications:
            ble_state.notifications[address] = []
        return {"message": f"Connected to {target.identifier()}", "address": address}
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Failed to connect: {str(e)}")

@app.post("/disconnect/{address}")
def disconnect(address: str):
    if address not in ble_state.peripherals:
        raise HTTPException(status_code=404, detail="Device not found")
    
    p = ble_state.peripherals[address]
    try:
        p.disconnect()
        # Clean up notifications
        if address in ble_state.notifications:
            del ble_state.notifications[address]
        return {"message": f"Disconnected from {p.identifier()}"}
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Failed to disconnect: {str(e)}")

@app.get("/device/{address}")
def get_device_info(address: str):
    if address not in ble_state.peripherals:
        raise HTTPException(status_code=404, detail="Device not found or not connected")
    
    p = ble_state.peripherals[address]
    if not p.is_connected():
        return {"address": address, "connected": False}

    services = []
    for s in p.services():
        chars = [c.uuid() for c in s.characteristics()]
        services.append({"uuid": s.uuid(), "characteristics": chars})

    return {
        "identifier": p.identifier(),
        "address": address,
        "connected": p.is_connected(),
        "mtu": p.mtu(),
        "services": services
    }

class WriteRequest(BaseModel):
    data: str  # Hex string or plain text? Let's assume hex string for binary data flexibility

@app.post("/device/{address}/read/{service_uuid}/{char_uuid}")
def read_characteristic(address: str, service_uuid: str, char_uuid: str):
    if address not in ble_state.peripherals:
        raise HTTPException(status_code=404, detail="Device not found")
    
    p = ble_state.peripherals[address]
    if not p.is_connected():
        raise HTTPException(status_code=400, detail="Device not connected")

    try:
        # SimplePyBLE read returns bytes
        data = p.read(service_uuid, char_uuid)
        # Return as hex string for safety
        return {"service_uuid": service_uuid, "char_uuid": char_uuid, "data_hex": data.hex(), "data_utf8": data.decode('utf-8', errors='ignore')}
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Read failed: {str(e)}")

@app.post("/device/{address}/write/{service_uuid}/{char_uuid}")
def write_characteristic(address: str, service_uuid: str, char_uuid: str, request: WriteRequest):
    if address not in ble_state.peripherals:
        raise HTTPException(status_code=404, detail="Device not found")
    
    p = ble_state.peripherals[address]
    if not p.is_connected():
        raise HTTPException(status_code=400, detail="Device not connected")

    try:
        # Expecting hex string input
        data_bytes = bytes.fromhex(request.data)
        p.write_request(service_uuid, char_uuid, data_bytes)
        return {"message": "Write successful"}
    except ValueError:
         raise HTTPException(status_code=400, detail="Invalid hex string")
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Write failed: {str(e)}")

@app.post("/device/{address}/write_command/{service_uuid}/{char_uuid}")
def write_command_characteristic(address: str, service_uuid: str, char_uuid: str, request: WriteRequest):
    if address not in ble_state.peripherals:
        raise HTTPException(status_code=404, detail="Device not found")
    
    p = ble_state.peripherals[address]
    if not p.is_connected():
        raise HTTPException(status_code=400, detail="Device not connected")

    try:
        # Expecting hex string input
        data_bytes = bytes.fromhex(request.data)
        p.write_command(service_uuid, char_uuid, data_bytes)
        return {"message": "Write command successful"}
    except ValueError:
         raise HTTPException(status_code=400, detail="Invalid hex string")
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Write command failed: {str(e)}")

@app.post("/device/{address}/notify/{service_uuid}/{char_uuid}")
def notify_characteristic(address: str, service_uuid: str, char_uuid: str):
    if address not in ble_state.peripherals:
        raise HTTPException(status_code=404, detail="Device not found")
    
    p = ble_state.peripherals[address]
    if not p.is_connected():
        raise HTTPException(status_code=400, detail="Device not connected")

    def notification_callback(data: bytes):
        # This runs in a separate thread, be careful with state
        # Appending to a list is generally thread-safe in Python (GIL), but explicit locking is safer if complex
        # For this simple case, simple append should be fine
        if address in ble_state.notifications:
            ble_state.notifications[address].append({
                "service": service_uuid,
                "characteristic": char_uuid,
                "data_hex": data.hex(),
                "data_utf8": data.decode('utf-8', errors='ignore'),
                "type": "notification"
            })

    try:
        p.notify(service_uuid, char_uuid, notification_callback)
        return {"message": "Subscribed to notifications"}
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Notify failed: {str(e)}")

@app.post("/device/{address}/indicate/{service_uuid}/{char_uuid}")
def indicate_characteristic(address: str, service_uuid: str, char_uuid: str):
    if address not in ble_state.peripherals:
        raise HTTPException(status_code=404, detail="Device not found")
    
    p = ble_state.peripherals[address]
    if not p.is_connected():
        raise HTTPException(status_code=400, detail="Device not connected")

    def indication_callback(data: bytes):
        if address in ble_state.notifications:
            ble_state.notifications[address].append({
                "service": service_uuid,
                "characteristic": char_uuid,
                "data_hex": data.hex(),
                "data_utf8": data.decode('utf-8', errors='ignore'),
                "type": "indication"
            })

    try:
        p.indicate(service_uuid, char_uuid, indication_callback)
        return {"message": "Subscribed to indications"}
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Indicate failed: {str(e)}")

@app.post("/device/{address}/unsubscribe/{service_uuid}/{char_uuid}")
def unsubscribe_characteristic(address: str, service_uuid: str, char_uuid: str):
    if address not in ble_state.peripherals:
        raise HTTPException(status_code=404, detail="Device not found")
    
    p = ble_state.peripherals[address]
    if not p.is_connected():
        raise HTTPException(status_code=400, detail="Device not connected")

    try:
        p.unsubscribe(service_uuid, char_uuid)
        return {"message": "Unsubscribed from notifications"}
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Unsubscribe failed: {str(e)}")

@app.get("/device/{address}/notifications")
def get_notifications(address: str):
    if address not in ble_state.notifications:
        return []
    
    # Return and clear notifications
    notifs = ble_state.notifications[address]
    ble_state.notifications[address] = []
    return notifs

def main():
    parser = argparse.ArgumentParser(description="SimpleAIBLE REST Server")
    parser.add_argument("--host", default="127.0.0.1", help="Host to bind to")
    parser.add_argument("--port", type=int, default=8000, help="Port to bind to")
    args = parser.parse_known_args()[0]
    
    uvicorn.run(app, host=args.host, port=args.port)

if __name__ == "__main__":
    main()
