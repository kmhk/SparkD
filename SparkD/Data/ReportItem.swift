//
//  ReportItem.swift
//  SparkD
//
//  Created by com on 6/9/20.
//  Copyright © 2020 com.sparkD. All rights reserved.
//

import UIKit

struct ReportItem {
    var id: String = ""
    var timestamp: Date = .init(timeIntervalSinceNow: 0)
    var title: String = ""
    var result1: Double = 0.0
    var result2: Double = 0.0
    var result3: Double = 0.0
    var result4: Double = 0.0
    var url1: String = ""
    var url2: String = ""
    var url3: String = ""
    var average: Double = 0.0
    var selected: Bool = false
    
    func getDict() -> [String: Any] {
        return ["id": id,
                "timestamp": timestamp.timeIntervalSinceNow,
                "dbId": timestamp.timeIntervalSinceNow,
                "title": title,
                "result1": result1,
                "result2": result2,
                "result3": result3,
                "result4": result4,
                "pictureUrl1": url1,
                "pictureUrl2": url2,
                "pictureUrl3": url3,
                "average": average,
                "selected": selected]
    }
}

struct TestDominantColor {
    static func getFirstColors() -> [Int]? {
        if let tmp = UserDefaults.standard.value(forKey: "TestMode") {
            let testArray = tmp as! [Any]
            return testArray[0] as? [Int]
        }
        
        return nil
    }
    
    static func getSecondColors() -> [Int]? {
        if let tmp = UserDefaults.standard.value(forKey: "TestMode") {
            let testArray = tmp as! [Any]
            return testArray[1] as? [Int]
        }
        
        return nil
    }
    
    static func getThirdColors() -> [Int]? {
        if let tmp = UserDefaults.standard.value(forKey: "TestMode") {
            let testArray = tmp as! [Any]
            return testArray[2] as? [Int]
        }
        
        return nil
    }
}
