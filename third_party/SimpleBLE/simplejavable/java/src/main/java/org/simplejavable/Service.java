package org.simplejavable;

import java.util.List;

public class Service {
    private final String uuid;
    private final List<Characteristic> characteristics;

    public Service(String uuid, List<Characteristic> characteristics) {
        this.uuid = uuid;
        this.characteristics = characteristics;
    }

    public String uuid() {
        return uuid;
    }

    public List<Characteristic> characteristics() {
        return characteristics;
    }
}