package org.simplejavable;

public class Descriptor {
    private final String uuid;

    public Descriptor(String uuid) {
        this.uuid = uuid;
    }

    public String uuid() {
        return uuid;
    }
}