//
//  Button.swift
//  SparkD
//
//  Created by com on 6/3/20.
//  Copyright © 2020 com.sparkD. All rights reserved.
//

import Foundation
import UIKit

extension UIButton {
    func roundSolidButton(title: String) {
        setTitle(title, for: .normal)
        setTitleColor(UIColor.black, for: .normal)
        
        backgroundColor = UIColor.groupTableViewBackground
        
        layer.cornerRadius = frame.height / 2
        layer.shadowOffset = CGSize(width: 1, height: 1)
        layer.shadowColor = UIColor.gray.cgColor
        layer.shadowOpacity = 1.0
        layer.masksToBounds = false
    }
    
    func roundGradientButton(title: String) {
        setTitle(title, for: .normal)
        setTitleColor(UIColor.white, for: .normal)
        
        let gradientLayer = CAGradientLayer()
        gradientLayer.frame = bounds
        gradientLayer.startPoint = .init(x: 0, y: 0)
        gradientLayer.endPoint = .init(x: 1, y: 0)
        gradientLayer.colors = [UIColor.purple.cgColor, UIColor.systemPink.cgColor]
        gradientLayer.cornerRadius = frame.height / 2
        layer.insertSublayer(gradientLayer, at: 0)
        
        layer.cornerRadius = frame.height / 2
        layer.shadowOffset = CGSize(width: 0, height: 2)
        layer.shadowColor = UIColor.gray.cgColor
        layer.shadowOpacity = 1.0
        layer.masksToBounds = false
    }
}
