//
//  Headset.swift
//
//

import Foundation
import SwiftUI

class Headset {
    init(boardId: BoardIds) throws {
        self.boardId = boardId
        
        do {
            try? BoardShim.logMessage(.LEVEL_INFO, "Headset.init() UUID: \(self.uuid) boardId: \(boardId)")
            self.params = BrainFlowInputParams(serial_port: Headset.scan(boardId))
            self.board = try BoardShim(boardId, params)
            self.samplingRate = try BoardShim.getSamplingRate(boardId)
            self.eegChannels = try BoardShim.getEEGchannels(boardId)
            self.markerChannel = try Int(BoardShim.getMarkerChannel(boardId))
            self.pkgIDchannel = try Int(BoardShim.getPackageNumChannel(boardId))
            self.timestampChannel = try Int(BoardShim.getTimestampChannel(boardId))
        }
        catch let bfError as BrainFlowException {
            try? BoardShim.logMessage (.LEVEL_ERROR, bfError.message)
            try? BoardShim.logMessage (.LEVEL_ERROR, "Error code: \(bfError.errorCode)")
            throw bfError
        }
        catch {
            try? BoardShim.logMessage (.LEVEL_ERROR, "undefined exception")
            throw error
        }
    }
    
    deinit {
        try? BoardShim.logMessage(.LEVEL_INFO, "Headset.deinit() UUID: \(self.uuid) boardId: \(self.boardId)")
        cleanup()
    }

    func writeStream(_ matrixRaw: [[Double]]) {
        guard (rawFile != nil) && (filteredFile != nil) else {
            try? BoardShim.logMessage(.LEVEL_ERROR, "data files are not open")
            return
        }
        
        let raw = rawFile!
        let filtered = filteredFile!
        
        do {
            let numSamples = matrixRaw[0].count
            let pkgIDs = matrixRaw[pkgIDchannel]
            let timestamps = matrixRaw[timestampChannel]
            let markers = matrixRaw[markerChannel]
            var rawSamples = [[Double]]()
            var filteredSamples = [[Double]]()
            for channel in eegChannels {
                let ch = Int(channel)
                var filtered = matrixRaw[ch].map { $0 / 24.0 }
                try DataFilter.removeEnvironmentalNoise(data: &filtered, samplingRate: samplingRate, noiseType: NoiseTypes.SIXTY)
                var rawSample = [Double]()
                var filteredSample = [Double]()
                for iSample in 0..<numSamples {
                    rawSample.append(matrixRaw[ch][iSample])
                    filteredSample.append(filtered[iSample])
                }
                rawSamples.append(rawSample)
                filteredSamples.append(filteredSample)
            }
            
            raw.writeEEGsamples(pkgIDs: pkgIDs, timestamps: timestamps, markers: markers, samples: rawSamples)
            filtered.writeEEGsamples(pkgIDs: pkgIDs, timestamps: timestamps, markers: markers, samples: filteredSamples)
        } catch let bfError as BrainFlowException {
            try? BoardShim.logMessage (.LEVEL_ERROR, bfError.message)
            try? BoardShim.logMessage (.LEVEL_ERROR, "Error code: \(bfError.errorCode)")
        } catch {
            try? BoardShim.logMessage (.LEVEL_ERROR, "cannot stream EEG samples to data files.\nError: \(error)")
        }
    }
    

    func cleanup() {
        try? BoardShim.logMessage(.LEVEL_INFO, "Headset.cleanup() UUID: \(self.uuid) boardId: \(self.boardId)")
        self.isActive = false
        self.isStreaming = false

        if let theBoard = board {
            try? theBoard.stopStream()
            try? theBoard.releaseSession()
        }
        
        if let rFile = self.rawFile  { rFile.synchronizeFile() }
        if let fFile = self.filteredFile  { fFile.synchronizeFile() }
    }
    
    static func scan(_ boardId: BoardIds) -> String {
        // Return the first device name matching "cu.usbserial-DM*"
        let fm = FileManager.default
        let prefix = "/dev"
        try? BoardShim.logMessage(.LEVEL_INFO, "Scanning for devices")

        do {
            let items = try fm.contentsOfDirectory(atPath: prefix)

            var deviceString: String = ""
            if cytons.contains(boardId) {
                deviceString = "cu.usbserial-DM" }
            else {
                deviceString = "cu.usbmodem"
            }

            for item in items.filter({$0.contains(deviceString)}) {
                try? BoardShim.logMessage(.LEVEL_INFO, "Found device \(item)")
                return prefix + "/" + item
            }
            
            try? BoardShim.logMessage(.LEVEL_ERROR, "Cannot find any matching devices")
            return ""
        } catch {
            try? BoardShim.logMessage(.LEVEL_ERROR, "Cannot list contents of /dev")
            return ""
        }
    }

}
