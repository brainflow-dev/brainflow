package org.simplejavable;

import java.util.List;

public class Characteristic {
    private final String uuid;
    private final List<Descriptor> descriptors;
    private final boolean canRead;
    private final boolean canWriteRequest;
    private final boolean canWriteCommand;
    private final boolean canNotify;
    private final boolean canIndicate;

    public Characteristic(String uuid, List<Descriptor> descriptors, boolean canRead,
                         boolean canWriteRequest, boolean canWriteCommand,
                         boolean canNotify, boolean canIndicate) {
        this.uuid = uuid;
        this.descriptors = descriptors;
        this.canRead = canRead;
        this.canWriteRequest = canWriteRequest;
        this.canWriteCommand = canWriteCommand;
        this.canNotify = canNotify;
        this.canIndicate = canIndicate;
    }

    public String uuid() {
        return uuid;
    }

    public List<Descriptor> descriptors() {
        return descriptors;
    }

    public boolean canRead() {
        return canRead;
    }

    public boolean canWriteRequest() {
        return canWriteRequest;
    }

    public boolean canWriteCommand() {
        return canWriteCommand;
    }

    public boolean canNotify() {
        return canNotify;
    }

    public boolean canIndicate() {
        return canIndicate;
    }
}