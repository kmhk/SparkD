//
//  View.swift
//  SparkD
//
//  Created by com on 6/3/20.
//  Copyright © 2020 com.sparkD. All rights reserved.
//

import Foundation
import UIKit

extension UIView {
    func roundGradientView() {
        let gradientLayer = CAGradientLayer()
        gradientLayer.frame = bounds
        gradientLayer.startPoint = .init(x: 0, y: 0)
        gradientLayer.endPoint = .init(x: 1, y: 0)
        gradientLayer.colors = [UIColor.purple.cgColor, UIColor.systemPink.cgColor]
        gradientLayer.cornerRadius = frame.width / 5
        layer.insertSublayer(gradientLayer, at: 0)
        
        layer.cornerRadius = frame.width / 5
        layer.shadowOffset = CGSize(width: 0, height: 2)
        layer.shadowColor = UIColor.gray.cgColor
        layer.shadowOpacity = 1.0
        layer.masksToBounds = false
    }
    
    func squareGradientView() {
        let gradientLayer = CAGradientLayer()
        gradientLayer.frame = bounds
        gradientLayer.startPoint = .init(x: 0, y: 0)
        gradientLayer.endPoint = .init(x: 1, y: 0)
        gradientLayer.colors = [UIColor.purple.cgColor, UIColor.systemPink.cgColor]
        layer.insertSublayer(gradientLayer, at: 0)
        
        layer.shadowOffset = CGSize(width: 0, height: 2)
        layer.shadowColor = UIColor.gray.cgColor
        layer.shadowOpacity = 1.0
        layer.masksToBounds = false
    }
}

extension UILabel {
    func roundGradientLabel() {
        let gradientLayer = CAGradientLayer()
        gradientLayer.frame = bounds
        gradientLayer.startPoint = .init(x: 0, y: 0)
        gradientLayer.endPoint = .init(x: 1, y: 0)
        gradientLayer.colors = [UIColor.purple.cgColor, UIColor.systemPink.cgColor]
        gradientLayer.cornerRadius = frame.width / 5
        layer.insertSublayer(gradientLayer, at: 0)
        
        layer.cornerRadius = frame.width / 5
        layer.shadowOffset = CGSize(width: 0, height: 2)
        layer.shadowColor = UIColor.gray.cgColor
        layer.shadowOpacity = 1.0
        layer.masksToBounds = false
    }
}

class RoundShadowView: UIView {
    override func awakeFromNib() {
        layer.cornerRadius = 10
        layer.shadowOffset = CGSize(width: 0, height: 2)
        layer.shadowColor = UIColor.gray.cgColor
        layer.shadowOpacity = 1.0
        layer.masksToBounds = false
    }
    
    override init(frame: CGRect) {
        super.init(frame: frame)
        
        layer.cornerRadius = 10
        layer.shadowOffset = CGSize(width: 0, height: 2)
        layer.shadowColor = UIColor.gray.cgColor
        layer.shadowOpacity = 1.0
        layer.masksToBounds = false
        
        backgroundColor = UIColor.white
    }
    
    required init?(coder: NSCoder) {
        super.init(coder: coder)
        
        layer.cornerRadius = 10
        layer.shadowOffset = CGSize(width: 0, height: 2)
        layer.shadowColor = UIColor.gray.cgColor
        layer.shadowOpacity = 1.0
        layer.masksToBounds = false
        
        backgroundColor = UIColor.white
    }
}
