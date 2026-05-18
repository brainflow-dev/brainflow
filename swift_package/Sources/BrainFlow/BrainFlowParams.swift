import Foundation

public struct BrainFlowInputParams: Codable, Equatable, Sendable {
    public var serial_port: String
    public var mac_address: String
    public var ip_address: String
    public var ip_address_aux: String
    public var ip_address_anc: String
    public var ip_port: Int
    public var ip_port_aux: Int
    public var ip_port_anc: Int
    public var ip_protocol: Int
    public var other_info: String
    public var timeout: Int
    public var serial_number: String
    public var file: String
    public var file_aux: String
    public var file_anc: String
    public var master_board: Int

    public init() {
        serial_port = ""
        mac_address = ""
        ip_address = ""
        ip_address_aux = ""
        ip_address_anc = ""
        ip_port = 0
        ip_port_aux = 0
        ip_port_anc = 0
        ip_protocol = IpProtocolTypes.NO_IP_PROTOCOL.rawValue
        other_info = ""
        timeout = 0
        serial_number = ""
        file = ""
        file_aux = ""
        file_anc = ""
        master_board = BoardIds.NO_BOARD.rawValue
    }

    public mutating func set_ip_protocol(_ ip_protocol: IpProtocolTypes) {
        self.ip_protocol = ip_protocol.rawValue
    }

    public mutating func set_master_board(_ board: BoardIds) {
        master_board = board.rawValue
    }

    public func to_json() throws -> String {
        try Self.encoder.encodeString(self)
    }

    private static let encoder: JSONEncoder = {
        let encoder = JSONEncoder()
        encoder.outputFormatting = [.sortedKeys]
        return encoder
    }()
}

public struct BrainFlowModelParams: Codable, Equatable, Sendable {
    public var metric: Int
    public var classifier: Int
    public var file: String
    public var other_info: String
    public var output_name: String
    public var max_array_size: Int

    public init(metric: Int, classifier: Int) {
        self.metric = metric
        self.classifier = classifier
        file = ""
        other_info = ""
        output_name = ""
        max_array_size = 8192
    }

    public init(metric: BrainFlowMetrics, classifier: BrainFlowClassifiers) {
        self.init(metric: metric.rawValue, classifier: classifier.rawValue)
    }

    public mutating func set_metric(_ metric: BrainFlowMetrics) {
        self.metric = metric.rawValue
    }

    public mutating func set_classifier(_ classifier: BrainFlowClassifiers) {
        self.classifier = classifier.rawValue
    }

    public func to_json() throws -> String {
        try Self.encoder.encodeString(self)
    }

    private static let encoder: JSONEncoder = {
        let encoder = JSONEncoder()
        encoder.outputFormatting = [.sortedKeys]
        return encoder
    }()
}

private extension JSONEncoder {
    func encodeString<T: Encodable>(_ value: T) throws -> String {
        let data = try encode(value)
        guard let string = String(data: data, encoding: .utf8) else {
            throw invalidArguments("Unable to encode JSON as UTF-8")
        }
        return string
    }
}
