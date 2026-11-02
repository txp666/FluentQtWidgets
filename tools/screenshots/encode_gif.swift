import Foundation
import ImageIO
import UniformTypeIdentifiers
// usage: swift encode_gif.swift <frames dir> <out.gif>
// Encodes NNN.png and delays.txt (ms per frame) from galleryshots into a GIF that loops forever.
let dir = CommandLine.arguments[1], out = CommandLine.arguments[2]
let delays = try! String(contentsOfFile: dir + "/delays.txt", encoding: .utf8)
    .split(separator: "\n").map { Double($0)! / 1000.0 }
let dst = CGImageDestinationCreateWithURL(URL(fileURLWithPath: out) as CFURL, UTType.gif.identifier as CFString,
                                          delays.count, nil)!
CGImageDestinationSetProperties(dst, [kCGImagePropertyGIFDictionary as String:
                                        [kCGImagePropertyGIFLoopCount as String: 0]] as CFDictionary)
for (i, d) in delays.enumerated() {
    let path = String(format: "%@/%03d.png", dir, i)
    let src = CGImageSourceCreateWithURL(URL(fileURLWithPath: path) as CFURL, nil)!
    let img = CGImageSourceCreateImageAtIndex(src, 0, nil)!
    CGImageDestinationAddImage(dst, img, [kCGImagePropertyGIFDictionary as String:
                                            [kCGImagePropertyGIFDelayTime as String: d,
                                             kCGImagePropertyGIFUnclampedDelayTime as String: d]] as CFDictionary)
}
print(CGImageDestinationFinalize(dst) ? "ok" : "failed")
