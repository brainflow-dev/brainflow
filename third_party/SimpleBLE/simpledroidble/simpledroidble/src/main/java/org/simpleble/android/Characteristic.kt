package org.simpleble.android

class Characteristic(
    val uuid: String,
    val descriptors: List<Descriptor>,
    val canRead: Boolean,
    val canWriteRequest: Boolean,
    val canWriteCommand: Boolean,
    val canNotify: Boolean,
    val canIndicate: Boolean
)