import Foundation

@main
struct testBandPower {
    static func main() throws {
        // use synthetic board for demo
        try BoardShim.enableDevBoardLogger()
        let params = BrainFlowInputParams()
        let boardId = BoardIds.SYNTHETIC_BOARD
        let boardDescription = try BoardShim.getBoardDescr(boardId)
        let samplingRate = boardDescription.sampling_rate
        let board = try BoardShim(boardId, params)
        try board.prepareSession()
        try board.startStream()
        try BoardShim.logMessage(.LEVEL_INFO, "start sleeping in the main thread")
        sleep(5)
        let nfft = try DataFilter.getNearestPowerOfTwo(samplingRate)
        let size = try board.getBoardDataCount()
        var data = try board.getBoardData(size)
        try board.stopStream()
        try board.releaseSession()
    
        let EEGchannels = boardDescription.eeg_channels 
        // second eeg channel of synthetic board is a sine wave at 10Hz, should see huge alpha
        let eegChannel = Int(EEGchannels[1])
        // optional detrend
        try DataFilter.deTrend(data: &data[eegChannel], operation: .LINEAR)
        
        let overlap = Int32(floor(Double(Int(nfft) / 2)))
        let psd = try DataFilter.getPSDwelch(data: data[eegChannel], nfft: nfft,
                                         overlap: overlap, samplingRate: samplingRate,
                                         window: .BLACKMAN_HARRIS)
    
        let bandPowerAlpha = try DataFilter.getBandPower(psd: psd, freqStart: 7.0, freqEnd: 13.0)
        let bandPowerBeta = try DataFilter.getBandPower(psd: psd, freqStart: 14.0, freqEnd: 30.0)
        print("alpha/beta:\(bandPowerAlpha / bandPowerBeta)")
    }
}
