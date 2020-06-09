//
//  global.swift
//  FireSparkSwift
//
//  Created by Pritesh Patel on 11/15/16.
//  Copyright Â© 2016 Spark Diagnostics. All rights reserved.
//

import Foundation

let maxColumns = 10
let maxRows = 30

enum StripType : Int {
    case VitD
    
    var description : String {
        return [.VitD: "VitD"][self]!
    }
    
    static func from(description: String) -> StripType {
        return ["VitD": .VitD][description]!
    }
    
}

struct StripData {
    var selectedStrip: StripType?
    var nTest:Int                            //g_nTest
    var nPattern = [Int]()                   //g_nPattern
    var xValueOrder = [Double]()
    var xValues : [Int: [Double]]
    var parameter: [String]
    var units: [String]
    var idealValues: [Int: [Double]]
    var textValues: [String : [(Double, String)]]
    var gSelector:Int
    var colorIndicatorValues :  [Int: [Double]]
    var mainColorPreset = [[String]]()
    
    init () {
        selectedStrip = nil
        nTest = 0
        nPattern = []
        xValueOrder = []
        xValues = [:]
        parameter = []
        units = []
        idealValues = [:]
        textValues = [:]
        gSelector = 0
        colorIndicatorValues = [:]
        mainColorPreset = [[String]]()
    }
}

struct GridRect {
    var x: Int
    var y: Int
    var width: Int
    var height: Int
}

struct Results {
    var rgbPattern = [[Int]]()
    var testT = Int()
    var controlC = Int()
    var betweenCT = Int()
    var histComp = [[Double]]()
    var mainRectXYWH = [[Int]]()
    var refRectXYWH = [[Int]]()
    var refPatternGridXYWH = [[GridRect]]()
    var rgbMain =  [Int]()
    var mainPatternGridXYWH = [GridRect]()
    var solver1Value = [Double]()
    var solver2Value = [Double]()
    var solver3Value = Double()
    var solver4Value = Double()
    var combinedValue = [Double]()
    var userValue = [Double]()
    var userColors = [UIColor]()
    
    init() {
        rgbPattern = [[Int]]()
        testT = Int()
        controlC = Int()
        betweenCT = Int()
        histComp = [[Double]]()
        mainRectXYWH = [[Int]]()
        refRectXYWH = [[Int]]()
        refPatternGridXYWH = [[GridRect]]()
        rgbMain =  [Int](repeating: 0, count: maxColumns)
        mainPatternGridXYWH = [GridRect]()
        solver1Value = [Double](repeating: 0.0, count: maxColumns)
        solver2Value = [Double](repeating: 0.0, count: maxColumns)
        combinedValue = [Double](repeating: 0.0, count: maxColumns)
        solver3Value = 0.0
        solver4Value = 0.0
        userValue = [Double](repeating: 0.0, count: maxColumns)
    }
}

var selectedStrip = StripType.VitD
var selStripData = StripData()
var nResult = Results()

var dbOrder = [Double](repeating: 0.0, count: maxColumns * maxRows)
var bDetection = false
var nOrder = 3

func generateStrip(strip: StripType) -> StripData {
    var data = StripData()
    
    switch strip {

    case .VitD:
        data.selectedStrip = .VitD
        data.nPattern = [6]
        data.nTest = 1
        data.xValues = [
            1: [5.0,10,20, 30, 50.0,100.0]
        ]
            
        data.parameter = ["VitD"]
        data.units = ["mg/ul"]
        data.idealValues = [
            1: [20.0, 50.0]
           ]
        data.textValues = ["VitD" : [(5.0,"Deficiency"), (10, "Deficiency"), (20, "Insufficient"), (30, "OK"), (50, "OK"), (70, "Good"), (100, "High")]]
        data.gSelector = 11
        data.mainColorPreset = [
            ["FBBD97", "F9AA8F", "F4858E", "A74860", "963A65", "782859"]
        ]
        data.colorIndicatorValues = [
            1: [5.0, 10.0, 20, 70, 100.0]
            ]
        
    }
    return data
}

func changeStrip(strip: StripType) {
    selStripData = generateStrip(strip: strip)
}

func configureNominal(for stripe: StripType) -> [[Double]] {
    switch stripe {
    
    case .VitD:
        return [[5, 10, 20, 50.0]]
        
    default:
        //        WARNING: UNTESTED BEHAVIOR. SHOULD BE REMOVED WITH ACTUAL NOMINAL VALUES
        let data = generateStrip(strip: stripe).xValues
        var mapped = [[Double]]()
        for key in data.keys.sorted() {
            mapped.append(data[key]!)
        }
        return mapped
    }
}


class GlobalHelper: NSObject {
    @objc class func fill(solver1: [Double], solver2: [Double]) {
        for value in solver1 {
            if value > 0 {
            nResult.combinedValue.append(value)
            }
        }
        for value in solver2 {
            if value > 0 {
                nResult.combinedValue.append(value)
            }
        }
        nResult.solver1Value = solver1
        nResult.solver2Value = solver2
    }
    
    @objc class func fill(solver3: Double, solver4:Double) {
           nResult.solver3Value = solver3
           nResult.solver4Value = solver4
       }
    
    
    @objc class func fill(testT: Int, controlC: Int, betweenCT: Int) {
        nResult.testT = testT
        nResult.controlC = controlC
        nResult.betweenCT = betweenCT
    }
    
    
    @objc class func fill(mainPatternXYWH: [[Int]], refPatternXYWH: [[Int]]) {
        nResult.mainRectXYWH = mainPatternXYWH
        nResult.refRectXYWH = refPatternXYWH
 
    }
    
    @objc class func fill(refPatternLocation: [String: Int]) {
        //       nResult.mainPatternLocation = refPatternXY
    }
    
    class func fill(nPattern: [Int]) {
        
    }
    
    @objc class func getCurrentSelectedStripe() -> Int {
        if selStripData.selectedStrip != nil {
            return selStripData.gSelector
        } else {
            return 0
        }
    }
    
    @objc class func getCurrentnTestStrip() -> Int {
        if selStripData.selectedStrip != nil {
    //        print(selStripData.nTest)
            return selStripData.nTest
        } else {
            return 0
        }
    }
    
    @objc class func getxValuesStrip(nPos: Int) -> [Double] {
        if selStripData.selectedStrip != nil {
            //         let newSelectStrip = generateStrip(strip: selStripData.selectedStrip!)
  //          print(selStripData.xValues[nPos + 1] ?? 0.0)
            return selStripData.xValues[nPos + 1]!
        } else {
            return [0.0]
        }
    }
    
    
    @objc class func getxValuesOrder(nPos: Int) -> [Double] {
        if selStripData.selectedStrip != nil {
            //         let newSelectStrip = generateStrip(strip: selStripData.selectedStrip!)
            //  print(selStripData.xValues[nPos + 1] ?? 0.0)
            return selStripData.xValueOrder
        } else {
            return [0.0]
        }
    }
    
    
    @objc class func getCurrentnPatternStrip() -> [Int] {
        if selStripData.selectedStrip != nil {
    //        print(selStripData.nPattern)
            return selStripData.nPattern
        } else {
            return []
        }
    }
    
    
}

